#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <time.h>
#include <poll.h>
#include <math.h>

#include "config.h"
#include "obfs.h"

_Static_assert(CHAR_BIT == 8, "Unsupported platform: byte sizes must be exactly 8 bits.");

typedef struct {
    substitution_t *substitution;
    permutation_t *perm;
} candidate_t;

typedef struct {
    candidate_t candidate;
    double score;
} scored_candidate_t;

typedef struct {
    int sockfd;
    struct sockaddr_in addr;
} sock_conn_t;

static inline int rng(int lo, int hi) {
    return lo + (int)((double)rand() / (RAND_MAX + 1.0) * (hi - lo + 1));
}

static inline double binom_pmf(int n, int k, double p)
{
    return exp(
        lgamma(n + 1)
      - lgamma(k + 1)
      - lgamma(n - k + 1)
      + k * log(p)
      + (n - k) * log(1.0 - p)
    );
}


static inline double binom_cdf(int n, int k, double p)
{
    if (k <= 0) return 1.0;
    if (k > n)  return 0.0;

    double prob = 0.0;

    /* start at P(X = k) */
    double term = binom_pmf(n, k, p);

    for (int i = k; i <= n; i++) {
        prob += term;

        /* compute P(X = i+1) from P(X = i) */
        term *= (double)(n - i) / (i + 1) * (p / (1.0 - p));
    }

    return prob;
}

int cmp_scores_desc(const void *a, const void *b) {
    const scored_candidate_t *sa = a;
    const scored_candidate_t *sb = b;

    if (sa->score < sb->score) return 1;
    if (sa->score > sb->score) return -1;
    return 0;
}

int generate_payload(char *buf, size_t size) {
    FILE *f = fopen("/dev/urandom", "rb");

    if (!f) return -1;

    fread(buf, 1, size, f);
    fclose(f);
    return 0;
}

void mutate(substitution_t *substitution, permutation_t *perm) {
    srand(time(NULL));

    if (rand() <= RAND_MAX / 2) {
        int a = rng(0, 256), b = rng(0, 256);
        while(b == a) b = rng(0, 256);

        substitution_t temp = substitution[a];
        substitution[a] = substitution[b];
        substitution[b] = temp;
    } else {
        *perm = (*perm + rng(0, 2048)) % 2^32;
    }
}

double do_trial(candidate_t *candidate, char *payload, size_t size, int n_tries, sock_conn_t *sock) {
    int fail = 0;

    uint8_t *dest = malloc(size);
    apply((const uint8_t*)payload, dest, size, candidate->substitution, candidate->perm, FORWARD);

    struct pollfd pfd = {
        .fd = sock->sockfd,
        .events = POLLIN
    };

    for (int i = 0; i < n_tries; i++) {
        sendto(sock->sockfd, dest, size, 0, (struct sockaddr*)&sock->addr, sizeof(sock->addr));

        pfd.revents = 0;
        int ret = poll(&pfd, 1, 100);

        if (ret > 0) {
            char buf[1];
            recvfrom(sock->sockfd, buf, sizeof(buf), 0, NULL, NULL);
        } 
        else if (ret == 0) {
            fail++;
        } else {
            perror("poll");
        }
    }

    free(dest);

    return binom_cdf(n_tries, fail, 0.05);
}

int main(void) {
    printf("kryptographer v0.1\n");
    printf("reading config file /etc/kryptographer/config.cfg\n");

    FILE *fp = fopen("/etc/kryptographer/config.cfg", "r");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    config_t config = {0};

    read_config(fp, &config);
    fclose(fp);

    candidate_t candidates[64];
    for (int i = 0; i < 64; i++) {
        candidate_t candidate = {
            malloc(sizeof(uint8_t) * 256),
            malloc(sizeof(int))
        };

        if (!candidate.substitution || !candidate.perm) {
            for (int j = 0; j < i; j++) {
                free(candidates[j].substitution);
                free(candidates[j].perm);
            }
            return -1;
        }

        gen_mapping(candidate.substitution);
        *candidate.perm = rng(0, 1024);

        candidates[i] = candidate;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(config.test_port)};
    inet_pton(AF_INET, config.url, &addr.sin_addr);

    sock_conn_t conn = {
        sock,
        addr
    };

    int best_idx = 0;

    for (int i = 1; i <= 10; i++) {
        size_t size = sizeof(char) * rng(5, 1480);
        char *payload = malloc(size);
        generate_payload(payload, size);
        //size_t size = 5;
        //char *payload = "apple";

        scored_candidate_t scores[64];
        double sum = 0;
        double best = -1;

        for (int j = 0; j < 64; j++) {
            scored_candidate_t score = { candidates[j], do_trial(&candidates[j], payload, size, config.payload_tests, &conn) };
            scores[j] = score;
            sum += score.score;
            if (score.score > best) {
                best = score.score;
                best_idx = j;
            }
        }

        qsort(scores, 64, sizeof(scored_candidate_t), cmp_scores_desc);

        for (int j = 8; j < 64; j++) {
            int elite_idx = rng(0, 8);
            memcpy(scores[j].candidate.substitution, scores[elite_idx].candidate.substitution, sizeof(uint8_t) * 256);
            memcpy(scores[j].candidate.perm, scores[elite_idx].candidate.perm, sizeof(int));

            mutate(scores[j].candidate.substitution, scores[j].candidate.perm);
        }

        printf("generation %d avg score: %.4f best score %.4f\n", i, sum/64, best);
        fflush(stdout);

        free(payload);
    }

    printf("the best idx is %d", best_idx);

    FILE *out_f = fopen("/etc/kryptographer/maps", "w");
    for (int i = 0; i < 256; i++) {
        fprintf(out_f, "%d ", (int) candidates[best_idx].substitution[i]);
    }
    fprintf(out_f, "%d", *candidates[best_idx].perm);
    fclose(out_f);

    for (int i = 0; i < 64; i++) {
        free(candidates[i].substitution);
        free(candidates[i].perm);
    }

    return 0;
}

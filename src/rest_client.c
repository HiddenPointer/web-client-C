/**
 * @file rest_client.c
 * @brief Implementación del cliente REST de alto rendimiento usando libcurl multi y cJSON
 */

#include "rest_client.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

struct rest_client {
    char *base_url;
    int max_handles;
    CURLM *multi;
    long connect_timeout;
    long timeout;
    int max_retries;    /**< Máximo reintentos */
    long backoff_ms;    /**< Backoff base en milisegundos */
    pthread_mutex_t lock;
};

// Función auxiliar para construir URL completa
static char *build_url(const char *base, const char *path) {
    size_t len = strlen(base) + strlen(path) + 2;
    char *url = malloc(len);
    if (!url) return NULL;
    strcpy(url, base);
    strcat(url, path);
    return url;
}

// Callback de escritura de libcurl
static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    rest_response *r = userdata;
    size_t total = size * nmemb;
    char *buf = realloc(r->body, r->size + total + 1);
    if (!buf) return 0;
    r->body = buf;
    memcpy(r->body + r->size, ptr, total);
    r->size += total;
    r->body[r->size] = '\0';
    return total;
}

struct rest_client *rest_create(const char *base_url, int max_handles) {
    struct rest_client *c = calloc(1, sizeof(*c));
    if (!c) return NULL;
    c->base_url = strdup(base_url);
    c->max_handles = max_handles;
    c->multi = curl_multi_init();
    c->connect_timeout = 30;
    c->timeout = 30;
    c->max_retries = 3; // valor predeterminado
    c->backoff_ms = 500; // valor predeterminado
    pthread_mutex_init(&c->lock, NULL);
    curl_multi_setopt(c->multi, CURLMOPT_MAXCONNECTS, max_handles);
    curl_multi_setopt(c->multi, CURLMOPT_PIPELINING, CURLPIPE_HTTP1|CURLPIPE_MULTIPLEX);
    return c;
}

void rest_client_set_timeouts(struct rest_client *c, long connect_timeout, long timeout) {
    c->connect_timeout = connect_timeout;
    c->timeout = timeout;
}

// Configurar reintentos ante errores transitorios (5xx)
void rest_client_set_retries(struct rest_client *c, int max_retries, long backoff_ms) {
    c->max_retries = max_retries;
    c->backoff_ms = backoff_ms;
}

/**
 * @brief Realizar una petición REST con reintentos ante errores 5xx (bloqueante).
 */
rest_response *rest_request(struct rest_client *c,
                            const char *method,
                            const char *path,
                            const char *json_body,
                            const struct rest_header *headers,
                            size_t n_headers) {
    int attempt = 0;
    long delay_ms = c->backoff_ms;
    rest_response *r = NULL;
    while (1) {
        struct curl_slist *hdrs = NULL;
        for (size_t i = 0; i < n_headers; i++) {
            char buf[512];
            snprintf(buf, sizeof(buf), "%s: %s", headers[i].key, headers[i].value);
            hdrs = curl_slist_append(hdrs, buf);
        }
        CURL *easy = curl_easy_init();
        char *url = build_url(c->base_url, path);
        if (!easy || !url) {
            if (hdrs) curl_slist_free_all(hdrs);
            return NULL;
        }
        rest_response *tmp = calloc(1, sizeof(*tmp));
        if (!tmp) {
            curl_easy_cleanup(easy);
            free(url);
            curl_slist_free_all(hdrs);
            return NULL;
        }
        tmp->body = malloc(1);
        tmp->size = 0;
        curl_easy_setopt(easy, CURLOPT_URL, url);
        curl_easy_setopt(easy, CURLOPT_HTTPHEADER, hdrs);
        curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(easy, CURLOPT_WRITEDATA, tmp);
        curl_easy_setopt(easy, CURLOPT_TIMEOUT, c->timeout);
        curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT, c->connect_timeout);
        if (json_body) {
            curl_easy_setopt(easy, CURLOPT_POSTFIELDS, json_body);
            curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, method);
        }
        // Ejecutar petición
        pthread_mutex_lock(&c->lock);
        curl_multi_add_handle(c->multi, easy);
        pthread_mutex_unlock(&c->lock);
        int still = 0;
        CURLMcode mcode;
        do { mcode = curl_multi_perform(c->multi, &still); }
        while (mcode == CURLM_CALL_MULTI_PERFORM);
        while (still) {
            curl_multi_wait(c->multi, NULL, 0, 1000, NULL);
            curl_multi_perform(c->multi, &still);
        }
        curl_multi_remove_handle(c->multi, easy);
        curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &tmp->status);
        curl_easy_cleanup(easy);
        free(url);
        curl_slist_free_all(hdrs);
        // parsear JSON
        tmp->json = cJSON_Parse(tmp->body);
        r = tmp;
        // verificar reintento
        if (r->status < 500 || r->status >= 600 || attempt >= c->max_retries) {
            break;
        }
        // preparar reintento
        rest_response_free(r);
        r = NULL;
        usleep(delay_ms * 1000);
        delay_ms *= 2;
        attempt++;
    }
    return r;
}

void rest_response_free(rest_response *r) {
    if (!r) return;
    free(r->body);
    cJSON_Delete(r->json);
    free(r);
}

void rest_destroy(struct rest_client *c) {
    if (!c) return;
    curl_multi_cleanup(c->multi);
    free(c->base_url);
    pthread_mutex_destroy(&c->lock);
    free(c);
}

#include <stdio.h>
#include "rest_client.h"

int main() {
    struct rest_client *c = rest_create("https://auth.example.com", 10);
    rest_client_set_timeouts(c, 5, 10);
    // Configurar reintentos para errores 5xx: 3 reintentos, backoff de 500ms
    rest_client_set_retries(c, 3, 500);

    // Ejemplo de cabeceras HTTP
    struct rest_header hdrs[] = {
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer TOKEN"}
    };

    // JSON de transacción de demostración
    const char *txn = "{\"amount\": 1000, \"currency\": \"USD\"}";
    rest_response *r = rest_request(c, "POST", "/v1/txn", txn, hdrs, 2);
    if (!r) {
        fprintf(stderr, "Request failed\n");
        return 1;
    }
    printf("Status: %ld\nBody: %s\n", r->status, r->body);
    if (r->json) {
        char *str = cJSON_Print(r->json);
        printf("JSON: %s\n", str);
        free(str);
    }
    rest_response_free(r);
    rest_destroy(c);
    return 0;
}

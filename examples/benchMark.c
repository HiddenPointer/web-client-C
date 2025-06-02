/**
 * @file obp_example.c
 * @brief Ejemplo de consumo del endpoint de cuenta de Open Bank Project
 */

 #include <stdio.h>
 #include <time.h>
 #include <stdlib.h>
 #include <string.h>
 #include "../src/rest_client.h"
 
 // Función para calcular la diferencia en segundos
 double diff_sec(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec)*1e-9;
}

 int main() {
     // Crear cliente apuntando al sandbox de Open Bank Project
     struct rest_client *c = rest_create("https://apisandbox.openbankproject.com", 10);
     rest_client_set_timeouts(c, 5, 10);
     rest_client_set_retries(c, 3, 500);
 
    const int N = 1000;
    struct timespec t0, t1;
    double total = 0;

     // Leer IDs y token desde variables de entorno
     const char *bank_id    = getenv("OBP_BANK_ID");
     const char *account_id = getenv("OBP_ACCOUNT_ID");
     const char *token_env  = getenv("OBP_TOKEN");
     if (!token_env) {
         fprintf(stderr, "Error: defina OBP_TOKEN\n");
         rest_destroy(c);
         return 1;
     }
     char token[256];
     strcpy(token, "DirectLogin token=");
     strcat(token, token_env);
     if (!bank_id || !account_id) {
         fprintf(stderr, "Error: defina OBP_BANK_ID y OBP_ACCOUNT_ID\n");
         rest_destroy(c);
         return 1;
     }
 
     // Construir ruta y cabecera Authorization
     char path[256];
     snprintf(path, sizeof(path),
              "/obp/v5.1.0/my/banks/%s/accounts/%s/account",
              bank_id, account_id);
     struct rest_header hdrs[] = {
         {"Authorization", token}
     };
 
    for(int i = 0; i < N; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        rest_response *r = rest_request(c, "GET", path, NULL, hdrs, 1);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        if (r) {
            total += diff_sec(t0, t1);
            rest_response_free(r);
        } else {
            fprintf(stderr, "La petición OBP falló\n");
            rest_destroy(c);
            return 1;
        }
    }
    printf("Requests: %d  Tiempo total: %.3fs  Latencia media: %.3fms\n",
           N, total, total/N*1e3);
    rest_destroy(c);
     return 0;
 }
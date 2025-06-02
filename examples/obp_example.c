/**
 * @file obp_example.c
 * @brief Ejemplo de consumo del endpoint de cuenta de Open Bank Project
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "../src/rest_client.h"
 #include "../src/lib/parson.h" // Agregar inclusión de Parson
 
 int main() {
     // Crear cliente apuntando al sandbox de Open Bank Project
     struct rest_client *c = rest_create("https://apisandbox.openbankproject.com", 10);
     rest_client_set_timeouts(c, 5, 10);
     rest_client_set_retries(c, 3, 500);
 
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
 
     // Petición GET
     rest_response *r = rest_request(c, "GET", path, NULL, hdrs, 1);
     if (!r) {
         fprintf(stderr, "La petición OBP falló\n");
         rest_destroy(c);
         return 1;
     }
 
     // Mostrar resultados
     printf("Código HTTP: %ld\n", r->status);
     printf("Cuerpo:\n%s\n", r->body);
     if (r->json) {
         // Serializar JSON a string con Parson
         char *str = json_serialize_to_string_pretty(r->json);
         printf("JSON parseado:\n%s\n", str);
         json_free_serialized_string(str);
     }
 
     rest_response_free(r);
     rest_destroy(c);
     return 0;
 }
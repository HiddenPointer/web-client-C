/**
 * @file rest_client.h
 * @brief Cliente REST de alto rendimiento usando libcurl multi y cJSON
 * @details API segura para hilos, no bloqueante, con conexiones persistentes y timeouts configurables.
 */
#ifndef REST_CLIENT_H
#define REST_CLIENT_H

#include <stddef.h>
#include <curl/curl.h>
#include <cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Par clave-valor para cabecera HTTP */
struct rest_header {
    char *key;   /**< Nombre de la cabecera */
    char *value; /**< Valor de la cabecera */
};

/** Manejador opaco del cliente REST */
struct rest_client;

/** Objeto de respuesta REST */
typedef struct rest_response {
    long status;      /**< Código de estado HTTP */
    char *body;       /**< Cuerpo de la respuesta (malloc'd) */
    size_t size;      /**< Número de bytes en el cuerpo */
    cJSON *json;      /**< JSON parseado o NULL en caso de error de parseo */
} rest_response;

/**
 * @brief Crear un cliente REST.
 * @param base_url URL base para peticiones (p.ej. "https://api.ejemplo.com").
 * @param max_handles Número máximo de conexiones concurrentes.
 * @return Puntero a rest_client o NULL en caso de error.
 */
struct rest_client *rest_create(const char *base_url, int max_handles);

/**
 * @brief Configurar timeouts del cliente.
 * @param c Manejador del cliente REST.
 * @param connect_timeout Timeout de conexión (segundos).
 * @param timeout Timeout total de operación (segundos).
 */
void rest_client_set_timeouts(struct rest_client *c, long connect_timeout, long timeout);

/**
 * @brief Configurar reintentos ante errores transitorios (5xx).
 * @param c Manejador del cliente REST.
 * @param max_retries Máximo número de reintentos.
 * @param backoff_ms Tiempo base de backoff en milisegundos.
 */
void rest_client_set_retries(struct rest_client *c, int max_retries, long backoff_ms);

/**
 * @brief Realizar una petición REST (bloqueante hasta completarse).
 * @param c Manejador del cliente REST.
 * @param method Método HTTP ("GET","POST","PUT","DELETE").
 * @param path Ruta URL anexada a base_url (p.ej. "/v1/txn").
 * @param json_body Cadena JSON para el cuerpo o NULL.
 * @param headers Array de cabeceras HTTP.
 * @param n_headers Número de cabeceras.
 * @return Puntero a rest_response o NULL en caso de error; liberar con rest_response_free().
 */
rest_response *rest_request(struct rest_client *c,
                            const char *method,
                            const char *path,
                            const char *json_body,
                            const struct rest_header *headers,
                            size_t n_headers);

/**
 * @brief Liberar una respuesta REST.
 * @param r Respuesta REST a liberar.
 */
void rest_response_free(rest_response *r);

/**
 * @brief Destruir un cliente REST y liberar recursos.
 * @param c Cliente REST a destruir.
 */
void rest_destroy(struct rest_client *c);

#ifdef __cplusplus
}
#endif

#endif /* REST_CLIENT_H */

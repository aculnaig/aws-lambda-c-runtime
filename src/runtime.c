#include "aws/lambda-c-runtime/runtime.h"

#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t read_header(char *buffer, size_t size, size_t nitems, void *userdata)
{
    char *aws_request_id = (char *) userdata;    
    char *token;
    while ((token = strsep(&buffer, ":")) != NULL)
        if (strcmp(token, "Lambda-Runtime-Aws-Request-Id") == 0)
            token = strsep(&buffer, ":");

    aws_request_id = malloc(strlen(token));

    strcpy(aws_request_id, token);

    return size * nitems;
}

static size_t read_body(char *buffer, size_t size, size_t nitems, void *userdata)
{
    aws_lambda_request_t *req = (aws_lambda_request_t *) userdata;
    req->payload = malloc(strlen(buffer));
    strcpy(req->payload, buffer);

    return size * nitems;
}

void run_handler(aws_lambda_response_t *(*handler)(aws_lambda_request_t *req))
{
    char url[128];
    snprintf(url, strlen(url), "http://%s/2018-06-01/runtime/invocation/next", getenv("AWS_LAMBDA_RUNTIME_API"));

    CURL *curl;
    char *aws_request_id = NULL;
    aws_lambda_request_t *req = malloc(sizeof(aws_lambda_request_t *));

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();

    while (1) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, read_header);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, aws_request_id);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_body);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, req);

        curl_easy_perform(curl);

        aws_lambda_response_t *res = handler(req);

        curl_easy_reset(curl);

        memset(url, 0, strlen(url));
        snprintf(url, strlen(url), "http://%s/2018-06-01/runtime/invocation/%s/response", getenv("AWS_LAMBDA_RUNTIME_API"), aws_request_id);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, res->payload);

        curl_easy_perform(curl);
    }

    free(aws_request_id);
    free(req);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

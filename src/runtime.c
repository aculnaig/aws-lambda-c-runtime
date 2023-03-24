#include "aws/lambda-c-runtime/runtime.h"

#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t read_header(char *buffer, size_t size, size_t nitems, void *userdata)
{
    struct curl_slist *headers = (struct curl_slist *) userdata;
    headers = curl_slist_append(headers, buffer);

    return size * nitems;
}

static size_t read_body(char *buffer, size_t size, size_t nitems, void *userdata)
{
    aws_lambda_request_t *req = (aws_lambda_request_t *) userdata;

    snprintf(req->payload, sizeof(buffer), "%s", buffer);

    return size * nitems;
}

void run_handler(aws_lambda_response_t *(*handler)(aws_lambda_request_t *req, aws_lambda_context_t *ctx))
{
    char url[128];
    snprintf(url, sizeof(url), "http://%s/2018-06-01/runtime/invocation/next", getenv("AWS_LAMBDA_RUNTIME_API"));

    CURL *curl;
    struct curl_slist *headers = curl_slist_append(NULL, NULL);
    aws_lambda_request_t *req = (aws_lambda_request_t *) malloc(sizeof(aws_lambda_request_t));
    aws_lambda_context_t *ctx = (aws_lambda_context_t *) malloc(sizeof(aws_lambda_context_t));

    ctx->function_name = getenv("AWS_LAMBDA_FUNCTION_NAME");
    ctx->function_version = getenv("AWS_LAMBDA_FUNCTION_VERSION");
    ctx->memory_limit_in_mb = atoi(getenv("AWS_LAMBDA_FUNCTION_MEMORY_SIZE"));
    ctx->log_group_name = getenv("AWS_LAMBDA_LOG_GROUP_NAME");
    ctx->log_stream_name = getenv("AWS_LAMBDA_LOG_STREAM_NAME");
    // TODO: Parse identity and client_context from headers
    ctx->identity = NULL;
    ctx->client_context = NULL;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();

    while (1) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, read_header);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_body);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, req);

        curl_easy_perform(curl);

	for (struct curl_slist *header = headers; header != NULL; header = header->next) {
            char *token = strsep(&header->data, ":");
            if (strcmp(token, "lambda-runtime-aws-request-id"))
                ctx->aws_request_id = strsep(&header->data, ":");
            else if (strcmp(token, "lambda-runtime-invoked-function-arn"))
                ctx->invoked_function_arn = strsep(&header->data, ":");
            else if (strcmp(token, "lambda-runtime-deadline-ms"))
                ctx->deadline_ms = strtol(strsep(&header->data, ":"), NULL, 10);
            else
            ;
	}

        aws_lambda_response_t *res = handler(req, ctx);

        curl_easy_reset(curl);

        memset(url, 0, strlen(url));
        snprintf(url, strlen(url), "http://%s/2018-06-01/runtime/invocation/%s/response", getenv("AWS_LAMBDA_RUNTIME_API"), ctx->aws_request_id);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, res->payload);

        curl_slist_free_all(headers);
        free(req->payload);

        curl_easy_perform(curl);
    }

    free(req);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

#include <aws/lambda-c-runtime/runtime.h>

#include <stdlib.h>
#include <string.h>

static aws_lambda_response_t *handler(aws_lambda_request_t *req)
{
    aws_lambda_response_t *res = malloc(sizeof(aws_lambda_response_t));

    strncpy(res->payload, req->payload, strlen(req->payload));

    return res;
}

int main(void)
{
    run_handler(handler);
}

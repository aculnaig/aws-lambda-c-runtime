#ifndef AWS_LAMBDA_C_RUNTIME
#define AWS_LAMBDA_C_RUNTIME

const char *const project_version = "0.1.0";

typedef struct invocation_request {

    char *payload;
} aws_lambda_request_t;

typedef struct invocation_response {

    char *payload;
    char *content_type;
} aws_lambda_response_t;

void run_handler(aws_lambda_response_t *(*handler)(aws_lambda_request_t *));

#endif AWS_LAMBDA_C_RUNTIME

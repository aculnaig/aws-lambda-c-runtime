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

typedef struct invocation_context {

    char *function_name;
    char *function_version;
    int memory_limit_in_mb;
    char *log_group_name;
    char *log_stream_name;
    char *aws_request_id;
    char *invoked_function_arn;
    long deadline_ms;
    char *identity;
    char *client_context;
} aws_lambda_context_t;

void run_handler(aws_lambda_response_t *(*handler)(aws_lambda_request_t *, aws_lambda_context_t *));

#endif // AWS_LAMBDA_C_RUNTIME

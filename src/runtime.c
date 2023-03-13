#include "aws/lambda-c-runtime/runtime.h"

#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void run_handler(aws_lambda_response_t *(*handler)(aws_lambda_request_t *req))
{
}

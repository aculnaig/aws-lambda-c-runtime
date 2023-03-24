FROM public.ecr.aws/lambda/provided:al2

COPY bootstrap ${LAMBDA_RUNTIME_DIR}
COPY lib ${LAMBDA_RUNTIME_DIR}/lib

ENTRYPOINT [ "/var/runtime/bootstrap" ]

CC = gcc

BUILD_DIR = build
SRC_DIR = src

SRC_Wildcard = ${SRC_DIR}/*.c
SRC_SV_wc = ${SRC_DIR}/server_*.c
SRC_CT_wc = ${SRC_DIR}/client_*.c

FLAGS = -lm -lpthread


all: client server

server: prereq
	@${CC} -o ${BUILD_DIR}/server ${SRC_SV_wc} ${FLAGS}	

client: prereq
	@${CC} -o ${BUILD_DIR}/client ${SRC_CT_wc} ${FLAGS}	

prereq:
	@mkdir -p ${BUILD_DIR}

clean: 
	@rm -fr ${BUILD_DIR}



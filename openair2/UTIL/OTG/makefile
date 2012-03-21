CC = gcc 
  
OBJ_OTG = main.c otg_tx.c otg_tx_socket.c net_data.c otg_rx.c ../MATH/oml.c ../../../openair1/PHY/CODING/crc_byte.c  


help: 
	@echo ' "make clean" to clean the generated file'
	@echo ' "make otg" to compile OTG'
	
otg: ${OBJ_OTG} 
	${CC} ${OBJ_OTG} -lm -o otg

clean: 
	@rm otg

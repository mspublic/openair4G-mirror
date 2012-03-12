CC = gcc 
  
OBJ_OTG = main.c otg_tx.c otg_tx_socket.c otg_rx.c ../MATH/oml.c ../MATH/taus.c otg.c otg_kpi.c


help: 
	@echo ' "make clean" to clean the generated file'
	@echo ' "make otg" to compile OTG'
	
otg: ${OBJ_OTG} 
	${CC} -pg -O3 ${OBJ_OTG} -lm -o otg

clean: 
	@rm otg

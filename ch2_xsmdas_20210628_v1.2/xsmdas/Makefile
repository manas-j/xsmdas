#this make file is for XSMDAS 
SRC_PATH=src

all:
	
	@echo "---------------------------------------------------------------------"
	@echo "XSMDAS: Data Analysis Software for Chandrayaan-II Solar X-ray Monitor"
	@echo "---------------------------------------------------------------------"	
	
	make -C $(SRC_PATH)/xsmgtigen	
	make -C $(SRC_PATH)/xsmgenspec
	make -C $(SRC_PATH)/xsmgenlc
	make -C $(SRC_PATH)/xsmaddspec
	
	@echo "-----------------------------------------------------------"
	@echo "Compilation of All modules of XSMDAS completed successfully"
	@echo "-----------------------------------------------------------"

full:
	 
	@echo "---------------------------------------------------------------------"
	@echo "XSMDAS: Data Analysis Software for Chandrayaan-II Solar X-ray Monitor"
	@echo "	           Full version including level-0 to level-1                "
	@echo "---------------------------------------------------------------------"   
	make -C $(SRC_PATH)/xsml1gen
	make -C $(SRC_PATH)/xsmhkextract
	make -C $(SRC_PATH)/xsmcalsa    
	make -C $(SRC_PATH)/xsmgtigen   
	make -C $(SRC_PATH)/xsmgenspec
	make -C $(SRC_PATH)/xsmgenlc
	make -C $(SRC_PATH)/xsmpds4gen
	make -C $(SRC_PATH)/xsmaddspec
	
	@echo "-----------------------------------------------------------"
	@echo "Compilation of All modules of XSMDAS completed successfully"
	@echo "-----------------------------------------------------------"

clean:
	
	make -C $(SRC_PATH)/xsmgtigen clean
	make -C $(SRC_PATH)/xsmgenspec clean
	make -C $(SRC_PATH)/xsmgenlc clean
	make -C $(SRC_PATH)/xsmaddspec clean

cleanfull:
	
	make -C $(SRC_PATH)/xsml1gen clean	
	make -C $(SRC_PATH)/xsmhkextract clean
	make -C $(SRC_PATH)/xsmcalsa clean
	make -C $(SRC_PATH)/xsmgtigen clean
	make -C $(SRC_PATH)/xsmgenspec clean
	make -C $(SRC_PATH)/xsmgenlc clean
	make -C $(SRC_PATH)/xsmpds4gen clean
	make -C $(SRC_PATH)/xsmaddspec clean

distclean:
	rm  bin/*

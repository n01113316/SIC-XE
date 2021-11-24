#include "headers.h"


int main( int argc, char* argv[]){

	//Main variable declaration

	FILE *fp;
	FILE *objFP;
	char line[SIZE];
	char *newsym;
	char *nexttoken;
	char *operand;
	char *operandCPY;
	char *oc1;//-
	char asciiHexString[SIZE*2] = {0};
	char objCode[SIZE] = {0};
	char hexString[SIZE*2]={0};
	char programStartName[7];
	int plusfinder[5000];
	int tagfinder[5000];
	int counter = 0;
	int startCounter = 0;
	int programLength = 0;
	int objLength = 0;
	int endRecordAddress = 0;
	int opTableIndex = 0;
	int currentIndex = 0;
	int startFlag = 0;
	int endFlag = 0;


	//2 Byte Register Numbers //-Ethan Coco
	/*
	int axe = 0;
	int xxe = 1;
	int lxe = 2;
	int bxe = 3;
	int sxe = 4;
	int txe = 5;
	int fxe = 6;
	int pcxe = 8;
	int swxe = 9;
	*/
	char *optok;//-Ethan Coco
	char *optok2;//-Ethan Coco



	struct Symbol **SymbolHashTable = malloc(SIZE * sizeof(SYMBOL *));
	struct TextRecord **TextRecords = malloc(SIZE * sizeof(TRecord *));

	struct ModificationRecord **ModRecords = malloc (SIZE * sizeof(MRecord *));

	struct HeaderRecord header;

	//Check to see if there are 2 arguments
	if ( argc != 2 ) {
	printf("ERROR: Usage: %s filename.\n", argv[0]);
	return 0;
	}

	//open the file in "read" only mode
	fp = fopen( argv[1], "r");

	//If file being opened brings back NULL, throw error
	if ( fp == NULL ) {
		printf("ERROR: %s could not be opened for reading.\n", argv[1] );
		exit(0);
	}

	initOpcodeTable(); // Build opcode table

	// Memory Allocation
	newsym = malloc(  SIZE * sizeof(char) );
	memset( newsym, '\0', SIZE * sizeof(char) );
	nexttoken = malloc(  SIZE * sizeof(char) );
	memset( nexttoken, '\0', SIZE * sizeof(char) );
	operand = malloc( SIZE * sizeof(char) );
	memset( operand, '\0', SIZE * sizeof(char) ); 
	operandCPY = malloc(SIZE * sizeof(char) );
	memset(operandCPY, '\0', SIZE * sizeof(char) );
	oc1 = malloc(SIZE * sizeof(char) );//-
        memset(oc1, '\0', SIZE * sizeof(char) );//-

	optok = malloc(SIZE * sizeof(char));//-Ethan Coco
	memset(optok,'\0',SIZE * sizeof(char));//-Ethan Coco
	optok2 = malloc(SIZE * sizeof(char));//-Ethan Coco
	memset(optok2,'\0',SIZE * sizeof(char));//-Ethan Coco
        //First pass
        while(  fgets( line , SIZE , fp ) != NULL   ) {
		if( counter >= 32768){
			printf("ERROR. PROGRAM DOES NOT FIT SIC MEMORY.\n");
			exit(0);
		}
		currentIndex = 0;

                //If the line starts with #, continue
                if ( line[0] == 35){
                        continue;
                }

                //If(first character is between A-Z)
                if (  ((line[0] >= 65 ) && ( line[0] <= 90 )) || line != NULL  )  {
			//tokenize the line using tabs and newline being delimiters.
                        newsym = strtok( line, " \t\n");
                        nexttoken = strtok( NULL, " \t\n"  ); // Gets next token by submitting null
                        operand = strtok( NULL, "\t\n" );

                        // error-checking
                        if(newsym == NULL && nexttoken == NULL){
                                printf("ERROR: SYMBOL & OPCODE/DIRECTIVE ARE EMPTY.");
                                exit(0);
                        }
			//->steven dear
			if(newsym[0]==43){
                                newsym[0] = '\n';
                                memmove(newsym,newsym+1,strlen(newsym));
                                printf("+ has been removed for %s\n", newsym);
                                plusfinder[counter]=1;
                        }
			if(nexttoken[0]==35){
				nexttoken[0] = '\n';
                                memmove(nexttoken,nexttoken+1,strlen(nexttoken));
                                printf("# has been removed for %s\n", nexttoken);
				tagfinder[counter]=1;
			}
			//<-end

                        if ( IsAValidSymbol(SymbolHashTable, newsym) == 0 ) {

                                printf("ERROR. %s IS AN  INVALID SYMBOL.\n", newsym);
                                exit(0);
                        }

                        //Check to see if first token is an opcode
			if( (IsAnOpcode(newsym) != -1)){
				operand = nexttoken;
				nexttoken = newsym;
				newsym = NULL;
				counter += 3;
				continue;
			}


			currentIndex = InsertSymbol(SymbolHashTable, newsym, &counter); //inserts new symbol into Hash Table

                        switch(IsADirective(nexttoken)){
				case 1: ; //START DIRECTIVE
					if(validHex(operand) == 0){
						printf("ERROR. INVALID HEX NUMBER. \n");
						exit(0);
					}
					if(Start(SymbolHashTable, currentIndex, operand, &counter)== 0){
						printf("ERROR. INVALID START NUMBER. \n");
						exit(0);
					}
					startFlag++;
					startCounter = counter;
				break;
                                case 2: //BYTE DIRECTIVE
					if(Byte(operand, &counter) == 0){
						printf("ERROR. ODD NUMBER OF HEXES. \n");
						exit(0);
					}
				break;
                                case 3:	//WORD DIRECTIVE
                                	if(atoi(operand) >= 8388607){
                                		printf("ERROR. WORD CONSTANT IS TOO LARGE. \n");
                                		exit(0);
                                	}

					Word(&counter);
                                break;
                                case 4: //RESB DIRECTIVE
					ResB(operand, &counter);
                                break;
                                case 5: //RESW DIRECTIVE
					ResW(operand, &counter);
                                break;
                                case 6: //RESR DIRECTIVE
					ResR(&counter);
                                break;
                                case 7: //EXPORTS DIRECTIVE
					Exports(&counter);
                                break;
				case 8: //END DIRECTIVE
					End(&counter);
					endFlag++;
				break;
				default :
		                        counter += 3;
				break;
			}
                }
	}

	if(startFlag != 1 || endFlag != 1){
		printf("ERROR. NONE/MORE THAN ONE START OR END DIRECTIVE PRESENT.\n");
		exit(0);
	}

	rewind(fp); //set file pointer to beginning of file
	programLength = counter - startCounter; //calculates length of the program
	counter = 0; //reset counter
	char *objFile = strcat(argv[1], ".obj"); //"filenamr.obj"
	objFP = fopen(objFile , "w");
	//PASS 2

        while(  fgets( line , SIZE , fp ) != NULL   ) {

                //If the line starts with #, continue
                if ( line[0] == 35){
                        continue;
                }

                //If(first character is between A-Z)
                if (  ((line[0] >= 65 ) && ( line[0] <= 90 )) || line != NULL  )  {
			//tokenize line
        		newsym = strtok( line, " \t\n"); //token 1
                	nexttoken = strtok( NULL, " \t\n"  ); //token 2
                	operand = strtok( NULL, "\t\n" ); //token 3

			//->steven dear
			if(newsym[0]==43){
                                newsym[0] = '\n';
                                memmove(newsym,newsym+1,strlen(newsym));
                        //        printf("+ has been removed for %s\n", newsym);
                        //        plusfinder[counter]=1;
                        }
			if(nexttoken[0]==35){
                                nexttoken[0] = '\n';
                                memmove(nexttoken,nexttoken+1,strlen(nexttoken));
                        //        printf("# has been removed for %s\n", nexttoken);
                        //        tagfinder[counter]=1;
			}
			//<-end
			currentIndex = SearchSymTab(SymbolHashTable, newsym); //Check if first token is a symbol
			if(currentIndex != -1){ //Symbol has been found
			}
			else if( strcmp(newsym, "RSUB") == 0){ //Check for RSUB opcode
				opTableIndex = IsAnOpcode(newsym); //find position in opcode table
				strcpy(objCode, OpcodeTable[opTableIndex].OpCode); //copy opcode hex
				strcat(objCode, "0000");
				insertTRecord(TextRecords, counter, OBJ_LENGTH, objCode);
				counter +=3;
				continue;
			}
			else if(onebXE(newsym)==1){//checks xe instructions if no symbol is present //-Ethan Coco
				opTableIndex = IsAnOpcode(newsym);
				strcpy(objCode,OpcodeTable[opTableIndex].OpCode);
				insertTRecord(TextRecords,counter,OBJ_LENGTH,objCode);
				counter +=1;
				continue;
			}
			else{	//if first token is not a symbol or RSUB opcode, rearrange token variables for consistency
				operand = nexttoken;
				nexttoken = newsym;
				newsym = NULL;
			}

			switch(IsADirective(nexttoken)){
				case 1: ; //START DIRECTIVE
					strcpy(header.name, newsym); //HEADER PROGRAM = First token
					header.startAddress = SymbolHashTable[currentIndex]->address; //HEADER STARTING ADDRESS = newsym.address
					header.length = programLength;	//HEADER LENGTH OF PROGRAM = programLength (caclulated after Pass 1)
					strcpy(programStartName, SymbolHashTable[currentIndex]->name); //programStartName = newsym.name (for Modification Records)
					counter = SymbolHashTable[currentIndex]->address; //update counter with symbol's address
				continue;
				case 2: //BYTE DIRECTIVE
					counter = SymbolHashTable[currentIndex]->address; //update counter with current symbol's address
					objLength = Byte2(operand, hexString); // remove char or hex indicator and return the length of the hex string
					insertTRecord(TextRecords, counter, objLength, hexString); //Insert into Text Records
				continue;
				case 3: //WORD DIRECTIVE
					counter = SymbolHashTable[currentIndex]->address; //update counter
					int x = atoi(operand); //convert operand into an int
					sprintf(asciiHexString,"%06X",x); //convert int operand to a hex string
					insertTRecord(TextRecords, counter, WORD_LENGTH, asciiHexString); //Insert into Text Records
				continue;
				case 4: //RESB DIRECTIVE
					counter = SymbolHashTable[currentIndex]->address; //update counter
				continue;
				case 5: //RESW DIRECTIVE
					counter = SymbolHashTable[currentIndex]->address; //update counter
				continue;
				case 6: //RESR DIRECTIVE
					counter = SymbolHashTable[currentIndex]->address; //update counter
				continue;
				case 7: //EXPORTS DIRECTIVE
					counter = SymbolHashTable[currentIndex]->address; //update counter
				continue;
				case 8: //END DIRECTIVE
					if(SearchSymTab(SymbolHashTable, operand) == -1){ //if symbol is not in SymbolHashTable, exit program
						printf("ERROR. SYMBOL IS NOT PRESENT IN SYMBOL TABLE.\n");
						exit(0);
					}
					endRecordAddress = SymbolHashTable[SearchSymTab(SymbolHashTable, operand)]->address; //endRecordAddress = Third token.address
					counter = SymbolHashTable[currentIndex]->address; //update counter
				continue;
				default :
				break;
			}
			if(onebXE(nexttoken)!=0){//checks xe instructions if symbol is present //-Ethan Coco
				if(onebXE(nexttoken)==1){//-Ethan Coco
					opTableIndex = IsAnOpcode(nexttoken);
					strcpy(objCode,OpcodeTable[opTableIndex].OpCode);
					insertTRecord(TextRecords,counter,OBJ_LENGTH,objCode);
					counter +=1;
				}else if(onebXE(nexttoken)==2){//-Ethan Coco
					opTableIndex = IsAnOpcode(nexttoken);
					strcpy(objCode,OpcodeTable[opTableIndex].OpCode);
					if(strcmp(nexttoken,"CLEAR")==0 || strcmp(nexttoken,"TIXR")==0){
						if(strcmp(operand,"A")==0){
							strcat(objCode,"0");
						}else if(strcmp(operand,"X")==0){
							strcat(objCode,"1");
						}else if(strcmp(operand,"L")==0){
							strcat(objCode,"2");
						}else if(strcmp(operand,"B")==0){
							strcat(objCode,"3");
						}else if(strcmp(operand,"S")==0){
							strcat(objCode,"4");
						}else if(strcmp(operand,"T")==0){
							strcat(objCode,"5");
						}else if(strcmp(operand,"F")==0){
							strcat(objCode,"6");
						}else if(strcmp(operand,"PC")==0){
							strcat(objCode,"8");
						}else if(strcmp(operand,"SW")==0){
							strcat(objCode,"9");
						}
						strcat(objCode,"0");
					}else if(strcmp(nexttoken,"SHIFTL")==0 || strcmp(nexttoken,"SHIFTR")==0){
						optok = strtok(operand,",");
						optok2 = strtok(NULL,"\t\n");

						if(strcmp(optok,"A")==0){
							strcat(objCode,"0");
						}else if(strcmp(optok,"X")==0){
							strcat(objCode,"1");
						}else if(strcmp(optok,"L")==0){
							strcat(objCode,"2");
						}else if(strcmp(optok,"B")==0){
							strcat(objCode,"3");
						}else if(strcmp(optok,"S")==0){
							strcat(objCode,"4");
						}else if(strcmp(optok,"T")==0){
							strcat(objCode,"5");
						}else if(strcmp(optok,"F")==0){
							strcat(objCode,"6");
						}else if(strcmp(optok,"PC")==0){
							strcat(objCode,"8");
						}else if(strcmp(optok,"SW")==0){
							strcat(objCode,"9");
						}
						strcat(objCode,optok2);
					}else if(strcmp(nexttoken,"SVC")==0){
						if(strcmp(operand,"A")==0){
							strcat(objCode,"0");
						}else if(strcmp(operand,"X")==0){
							strcat(objCode,"1");
						}else if(strcmp(operand,"L")==0){
							strcat(objCode,"2");
						}else if(strcmp(operand,"B")==0){
							strcat(objCode,"3");
						}else if(strcmp(operand,"S")==0){
							strcat(objCode,"4");
						}else if(strcmp(operand,"T")==0){
							strcat(objCode,"5");
						}else if(strcmp(operand,"F")==0){
							strcat(objCode,"6");
						}else if(strcmp(operand,"PC")==0){
							strcat(objCode,"8");
						}else if(strcmp(operand,"SW")==0){
							strcat(objCode,"9");
						}
						strcat(objCode,"0");
					}else{
						optok = strtok(operand,",");
						optok2 = strtok(NULL,"\t\n");

						if(strcmp(optok,"A")==0){
							strcat(objCode,"0");
						}else if(strcmp(optok,"X")==0){
							strcat(objCode,"1");
						}else if(strcmp(optok,"L")==0){
							strcat(objCode,"2");
						}else if(strcmp(optok,"B")==0){
							strcat(objCode,"3");
						}else if(strcmp(optok,"S")==0){
							strcat(objCode,"4");
						}else if(strcmp(optok,"T")==0){
							strcat(objCode,"5");
						}else if(strcmp(optok,"F")==0){
							strcat(objCode,"6");
						}else if(strcmp(optok,"PC")==0){
							strcat(objCode,"8");
						}else if(strcmp(optok,"SW")==0){
							strcat(objCode,"9");
						}

						if(strcmp(optok2,"A")==0){
							strcat(objCode,"0");
						}else if(strcmp(optok2,"X")==0){
							strcat(objCode,"1");
						}else if(strcmp(optok2,"L")==0){
							strcat(objCode,"2");
						}else if(strcmp(optok2,"B")==0){
							strcat(objCode,"3");
						}else if(strcmp(optok2,"S")==0){
							strcat(objCode,"4");
						}else if(strcmp(optok2,"T")==0){
							strcat(objCode,"5");
						}else if(strcmp(optok2,"F")==0){
							strcat(objCode,"6");
						}else if(strcmp(optok2,"PC")==0){
							strcat(objCode,"8");
						}else if(strcmp(optok2,"SW")==0){
							strcat(objCode,"9");
						}
					}
					insertTRecord(TextRecords,counter,OBJ_LENGTH,objCode);
					counter +=2;

				}else if(onebXE(nexttoken)==3){//-Ethan Coco
					opTableIndex = IsAnOpcode(nexttoken); //Find opcode within OpCodeTable
					strcpy(objCode, OpcodeTable[opTableIndex].OpCode); //copy opcode hex
					strcpy(operandCPY, operand); //copy third token to avoid unintentional modification of original
					sprintf(hexString,"%X", SymbolHashTable[SearchSymTab(SymbolHashTable, operand)]->address); //find referenced symbol, pull the address, convert address to hex, output to "hexString" variable
					strcat(objCode,hexString);
					if(newsym == NULL){
						counter += 3;
						insertTRecord(TextRecords,counter,OBJ_LENGTH,objCode);
						//MRecord
					}else{
						counter = SymbolHashTable[currentIndex]->address;
						insertTRecord(TextRecords,counter,OBJ_LENGTH,objCode);
						//MRecord
					}
				}
				continue;
			}else{
			opTableIndex = IsAnOpcode(nexttoken); //Find opcode within OpCodeTable
			strcpy(objCode, OpcodeTable[opTableIndex].OpCode); //copy opcode hex
			strcpy(operandCPY, operand); //copy third token to avoid unintentional modification of original
			}
			//copy opcode
			//strcpy(oc1,objCode);//-

			//check for indirect addressing
			if(strstr(operandCPY, ",X") != NULL){
				char *t = strtok(operand, ",X");
				if(SearchSymTab(SymbolHashTable, t) == -1){
					printf("ERROR. SYMBOL %s IS NOT PRESENT IN SYMBOL TABLE.\n", t);
					exit(0);
				}
				int indirectAddress = SymbolHashTable[SearchSymTab(SymbolHashTable, t)]->address + 32768; //32768 = 8000 in hex
				sprintf(hexString, "%X", indirectAddress);
			}
			else{
				if(SearchSymTab(SymbolHashTable, operand) == -1){
					printf("ERROR. SYMBOL %s IS NOT PRESENT IN SYMBOL TABLE.\n", operand);
					exit(0);
				}
				sprintf(hexString,"%X", SymbolHashTable[SearchSymTab(SymbolHashTable, operand)]->address); //find referenced symbol, pull the address, convert address to hex, output to "hexString" variable
			}
			strcat(objCode,hexString); // add hexstring result to end of objCode

			if(newsym == NULL){
				counter += 3;
				insertTRecord(TextRecords,counter,OBJ_LENGTH,objCode);
				//MRecord
			}else{
				counter = SymbolHashTable[currentIndex]->address;
				insertTRecord(TextRecords,counter,OBJ_LENGTH,objCode);
				//MRecord
			}
		//}
	}
}

	for(int i=0; i<SIZE; i++){//Prints only object code for the time being, will change //-Ethan Coco
		if(TextRecords[i])
			fprintf(objFP,"%s\n",TextRecords[i]->objectCode);
	}

	//Closing statements
	fclose( fp );
	fclose( objFP );
	free(SymbolHashTable);
	free(TextRecords);
	free(ModRecords);

	return 0;
}

int validHex(char *test){ //check if string is a valid hex string
	int length = strlen(test);
	for(int i=0; i<length; i++){
		if(! (isxdigit(test[i]) ) )
			return 0;
	}
	return 1;
}

//function for checking if an opcode belongs to a 1-byte instruction //-Ethan Coco
int onebXE(char *objCode){
	int check = 0;
	if(strcmp(objCode,"FIX")==0 || strcmp(objCode,"FLOAT")==0 || strcmp(objCode,"HIO")==0 || strcmp(objCode,"NORM")==0 || strcmp(objCode,"SIO")==0 || strcmp(objCode,"TIO")==0){
		check = 1;
	}else if(strcmp(objCode,"ADDR")==0 || strcmp(objCode,"CLEAR")==0 || strcmp(objCode,"COMPR")==0 || strcmp(objCode,"DIVR")==0 || strcmp(objCode,"MULR")==0 || strcmp(objCode,"RMO")==0 || strcmp(objCode,"SHIFTL")==0 || strcmp(objCode,"SHIFTR")==0 || strcmp(objCode,"SUBR")==0 || strcmp(objCode,"SVC")==0 || strcmp(objCode,"TIXR")==0){
		check = 2;
	}else if(strcmp(objCode,"ADDF")==0 || strcmp(objCode,"COMPF")==0 || strcmp(objCode,"DIVF")==0 || strcmp(objCode,"LDB")==0 || strcmp(objCode,"LDF")==0 || strcmp(objCode,"LDS")==0 || strcmp(objCode,"LDT")==0 || strcmp(objCode,"LPS")==0 || strcmp(objCode,"MULF")==0 || strcmp(objCode,"SSK")==0 || strcmp(objCode,"STB")==0 || strcmp(objCode,"STF")==0 || strcmp(objCode,"STI")==0 || strcmp(objCode,"STS")==0 || strcmp(objCode,"STT")==0 || strcmp(objCode,"SUBF")==0){
		check = 3;//
	}
	return check;
}

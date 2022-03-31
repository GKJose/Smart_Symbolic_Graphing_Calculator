/**
	AUTHOR: JOSE MENDEZ
*/


#include <Keypad.hxx>

#if ENABLE_MCP_KEYPAD

Keypad::Keypad(){
	// Create the MCP23017 driver.
	mcp = MCP23017(BUS,ADDRESS);
	//Set the button states to zero.
	buttonStates = 0;	
	//Open the I2C connection to the button matrix
	mcp.openI2C();
	
	//Initalize the MCP23017 pins
	for(int i = 0; i < N_COLS; i++){
	mcp.pinMode(cols[i],OUTPUT);
	}
	for(int i = 0; i < N_ROWS; i++){
	mcp.pinMode(rows[i],INPUT);
	}

	
}

Keypad::~Keypad(){
	//Close the I2C connection once this object is destoried.
	mcp.closeI2C();
}

/**
	FUNCTION NAME: poll
	DESCRIPTION: Checks if any buttons were pressed and saves it in a bit string. 
*/
void Keypad::poll(){
        //Cycle through the columns, setting one column at a time to high
	for(int c = 0; c < N_COLS; c++){
		mcp.digitalWrite(cols[c],HIGH);
		uint64_t data = mcp.readGPIO(1);
		buttonStates |= data << 8*c;
		mcp.digitalWrite(cols[c],LOW);
		
	}

}
/**
	FUNCTION NAME: isPressed
	DESCRIPTION: Checks if the bit associated with the button is set.
	PARAMETERS: id - the button id
	RETURNS: true if the button was pressed, false otherwise.
*/
bool Keypad::isPressed(uint8_t id){
	//Check if the bit associated with the button is set
	if(buttonStates & ( 1ULL << id)){
		//Set the button state to zero once we have processed it.
		buttonStates = buttonStates & ~(1ULL << id);
		return true;
	}
	return false;
}
bool Keypad::isBothPressed(uint8_t id1,uint8_t id2){
	return ( buttonStates & ( 1ULL << id1) ) & ( buttonStates & ( 1ULL << id2) );
}
void Keypad::reset(){
	buttonStates = 0;
}

#endif

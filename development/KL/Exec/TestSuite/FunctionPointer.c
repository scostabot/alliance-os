int functionCalled(void)

{
	return(0x5A);
}

int main()

{
	int (*functionPointer)();

	functionPointer=functionCalled;
	return((*functionPointer)());
}


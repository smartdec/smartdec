
int main(int argc, char **argv) {
	try {
		argc++;
		
		if(argc == 10)
			throw int();
		
		argc++;
		
		try {
			argc += 5;
			
			if(argc == 7)
				throw long();
				
			argc ++;
		} catch(long) {
			argc += 11;
		}
		
		argc += 2;
	} catch (int) {
		argc++;
	} catch (long) {
		argc += 10;
	}
	
	return argc;
}
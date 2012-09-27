package dk.aau.sw1001f12;

public class Logger {
	
	public static void Log(String message) {
		org.apache.log4j.Logger.getLogger("THAPS").info(message);
	}

}

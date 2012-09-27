package dk.aau.sw1001f12;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;

public class Template {

	private static Template instance;
	ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
	
	public static Template getInstance() {
		if (instance == null)
			instance = new Template();
		return instance;
	}
	
	public String readTemplate(String template) {
		
		try {
			StringBuilder content = new StringBuilder();
			
			InputStream is = classLoader.getResourceAsStream("templates/" + template + ".html");
			BufferedReader reader = new BufferedReader(new InputStreamReader(is));
			
			String line;
			
			while ((line = reader.readLine()) != null)
				content.append(line);
			
			reader.close();
			is.close();
			
			return content.toString();
		} catch (Exception e) {
			e.printStackTrace();
			return "Unable to read template file. Error message: " + e.getMessage();
		}
	}
	
}

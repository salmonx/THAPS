package dk.aau.sw1001f12;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

public class Configuration {
	
	private static Configuration instance;
	private Properties props;
	
	private Configuration() {}
	
	public static Configuration getInstance() {
		if (instance == null)
			instance = new Configuration();
		return instance;
	}
	
	public void readConfiguration() throws FileNotFoundException, IOException {
		String jarPath = Configuration.class.getProtectionDomain().getCodeSource().getLocation().getPath();
		jarPath = jarPath.substring(0, jarPath.lastIndexOf('/') + 1);
		props = new Properties();
		props.load(new FileInputStream(jarPath + File.separator + "configuration.ini"));
	}
	
	public int getInt(String conf) {
		try {
			return Integer.parseInt(props.getProperty(conf));
		} catch (NumberFormatException e) {
			return -1;
		}
	}
	
	public String getString(String conf) {
		String s = props.getProperty(conf);
		return (s == null) ? "" : s;
	}
	
}

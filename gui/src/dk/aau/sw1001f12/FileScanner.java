package dk.aau.sw1001f12;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;

public class FileScanner {
	
	ArrayList<String> files;
	ArrayList<String> scannedFiles;
	
	public FileScanner() throws IOException {
		
		files = new ArrayList<String>();
		scannedFiles = new ArrayList<String>();
		
		if (GUI.getInstance().getDocumentRoot() != null) {	

			File dir = new File(GUI.getInstance().getDocumentRoot());
			
			if (!dir.isDirectory())
				throw new IOException("Document root '"+ dir.getAbsolutePath() +"' is not a directory");
			
	    	
	    	DBCollection coll = Database.getInstance().getDB("local").getCollection("thaps");
			DBCursor record = coll.find(new BasicDBObject("pid", GUI.getInstance().getPID()));
			
			while (record.hasNext())
				scannedFiles.add(record.next().get("filename").toString());
			
			scanDirectory(dir);
		}
	}
	
	public ArrayList<String> getFiles() {
		return files;
	}

	private void scanDirectory(File directory) {
		
		File[] dirFiles = directory.listFiles();
		
		for (int i = 0; i < dirFiles.length; i++) {
			
			if (dirFiles[i].isDirectory()) {
				scanDirectory(dirFiles[i]);
			} else if (dirFiles[i].getName().endsWith(".php")) {
				String path = dirFiles[i].getPath();
				if (!scannedFiles.contains(path)) {
					path = path.replaceFirst(GUI.getInstance().getDocumentRoot(), "");
					path = GUI.getInstance().getHostPath() +"/"+ path; 
					files.add(path);
				}
			}
			
		}
		
	}

}

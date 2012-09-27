package dk.aau.sw1001f12;

import java.net.UnknownHostException;

import com.mongodb.Mongo;
import com.mongodb.MongoException;

import static dk.aau.sw1001f12.Logger.Log;

public class Database extends Mongo {
	
	private static Database instance;
	private boolean connected = false;

	public Database() throws UnknownHostException, MongoException {
		super(Configuration.getInstance().getString("db.host"), Configuration.getInstance().getInt("db.port"));
		if (!getDB("local").authenticate(Configuration.getInstance().getString("db.user"), Configuration.getInstance().getString("db.pass").toCharArray())) {
			close();
			throw new MongoException("Invalid credentials");
		} else {
			connected = true;
		}
	}
	
	public synchronized static Database getInstance() {
		if (instance == null) {
			try {
				instance = new Database();
			} catch (UnknownHostException e) {
				Log("Unable to instantiate database connection");
				e.printStackTrace();
			} catch (MongoException e) {
				Log("Unable to instantiate database connection");
				e.printStackTrace();
			}
		}
		return instance;
	}
	
	public boolean isConnected() {
		return connected;
	}
	
	public void disconnect() {
		if (connected) {
			connected = false;
			instance = null;
			close();
		}
	}

}

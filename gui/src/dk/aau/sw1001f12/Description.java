package dk.aau.sw1001f12;

import java.util.ArrayList;

public class Description {

	private ArrayList<String> flowpaths = new ArrayList<String>();
	private ArrayList<String> dependencies = new ArrayList<String>();

	public ArrayList<String> getFlowpath() {
		return flowpaths;
	}

	public void setFlowpath(ArrayList<String> flowpaths) {
		this.flowpaths = flowpaths;		
	}
	
	public ArrayList<String> getDependencides() {
		return dependencies;
	}

	public void setDependencies(ArrayList<String> dependencies) {
		this.dependencies = dependencies;		
	}
	
}

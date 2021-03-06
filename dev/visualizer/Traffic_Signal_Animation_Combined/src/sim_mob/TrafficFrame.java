package sim_mob;

import java.awt.*; 
import java.io.IOException;

import javax.swing.*; 

public class TrafficFrame extends JFrame{

	private static final long serialVersionUID = 1L;

	public static void main(String[] args) throws IOException {
		String inputFile = args.length>0 ? args[0] : "log4.out";
		
		TrafficFrame frame = new TrafficFrame(inputFile);
		
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE); 
		
		frame.setVisible(true); 
	}
	
	
	// Constructor
	public TrafficFrame(String inFileName) throws IOException { 

		// Set title
		super("Traffic Signal Animation");  
		
		// Set size
		setSize(1200, 800); 
		TrafficPanel trafficSignal = new TrafficPanel(inFileName); 
		Container contentPane= getContentPane();   
		contentPane.add(trafficSignal);   
	} 
	
}

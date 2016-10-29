package example;

import jrapl.EnergyCheckUtils;

public class Test {
	public static void main(String[] args) {

		double[] before = EnergyCheckUtils.getEnergyStats();
		try {
			Thread.sleep(10000);
		} catch(Exception e) {
		}
		double[] after = EnergyCheckUtils.getEnergyStats();
		for(int i = 0; i < EnergyCheckUtils.socketNum; i++) {
			System.out.println("Power consumption of dram: " + (after[0] - before[0]) / 10.0 + " power consumption of cpu: " + (after[1] - before[1]) / 10.0 + " power consumption of package: " + (after[2] - before[2]) / 10.0);
		}
		EnergyCheckUtils.ProfileDealloc();
	}
	
}
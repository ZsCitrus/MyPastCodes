import java.util.*;
import java.io.*;
/*
* A2ZhouXing.java
* COMP 2140 SECTION A01
* INSTRUCTOR Cameron (A01)
* ASSIGNMENT Assignment 2
* Name:Xing Zhou
* Student Number:7869781
* Compeletion date:Oct 26
* 
* PURPOSE: Modelling an Airport Baggage Conveyor Belt with a Linked List
*/

public class A2ZhouXing{
  private static final String[] COMMAND_SET={"CHECKBAGS","PRINTSUMMARY","PRINTDETAIL","LOADFLIGHT","REMOVEOVERSIZE"};
  private static ConveyorBelt bagList=new ConveyorBelt();
  private static int bagCount=0;
    
  public static void main(String[] args){
    processFileInput();
    System.out.println( "\nThis the end of the program.");
  }//end main function

  public static void processFileInput(){
    Scanner keyboard;
    String fileName;
    FileReader fileInput;
    BufferedReader line;
    System.out.println( "\nEnter the input file name (txt files only): ");
    keyboard=new Scanner(System.in);  
    fileName=keyboard.nextLine();  
    try{
      fileInput=new FileReader(fileName);
      line=new BufferedReader(fileInput);
    }
    catch(IOException e){
      System.out.println("enconter problems when reading file");
    }
    do{
      String lineContent=line.readLine();
      if(lineContent!=null){
        Scanner readCommand=new Scanner(lineContent);
        String command=readCommand.next();
        if(command.equals(COMMAND_SET[0])){
        }
        processCommand(lineContent);
      }
    }while(lineContent!=null);
    line.close();
  }//end processFileInput

  private static void processCommand(String s){
    Scanner readCommand=new Scanner(s);
    String command=readCommand.next();
    if(readCommand.hasNextInt()){
      int flight=readCommand.nextInt();
      double weight=readCommand.nextDouble();
      double size=readCommand.nextDouble();
      boolean vip=readCommand.nextBoolean();
      Baggage bagCheckedin=new Baggage(flight, weight, size, vip);
      if(vip)
        bagList.addVIP(bagCheckedin);
      else
        bagList.addToEnd(bagCheckedin);
    }
    
    String command=readCommand.next();
    if(command.equals(COMMAND_SET[0])){
      bagCount+=readCommand.nextInt();
    }
    
    if(command.equals(COMMAND_SET[1])){
      if(bagCount==ConveyorBelt.readCount())
        bagList.printSummary();
      else
        System.out.println("something wrong with counting");
    }
    
    if(command.equals(COMMAND_SET[2])){
     if(bagCount==ConveyorBelt.readCount())
        bagList.printDetail();
      else
        System.out.println("something wrong with counting");
    }
    
    if(command.equals(COMMAND_SET[3])){
      int num=readCommand.nextInt();
      int load=bagList.loadFlight(num);
      bagCount-=load;
    }
    
    if(command.equals(COMMAND_SET[4])){
      double num=readCommand.nextDouble();
      int remoeve=bagList.removeOversize(num);
      bagCount-=remove;
    }
  }//end processCommand
}//end Main(Assignment) class
 
class Baggage{
  private int flightNo;
  private double bagWeight;
  private double bagSize;
  private boolean isVIP;
  
  public Baggage(int flight, double weight, double size, boolean vip){
    flightNo=flight;
    bagWeight=weight;
    bagSize=size;
    isVIP=vip;
  }
 
  public int getFlight(){return flightNo;}
  public double getWeight(){return bagWeight;}
  public double getSize(){return bagSize;}
  public boolean getStatus(){return isVIP;}
}//end class Baggage

class BagNode{
  private Baggage bagItem;
  private BagNode ahead;
  private BagNode back;
     
  public BagNode(Baggage newItem, BagNode theAhead, BagNode theAhead){
    bagItem=newItem;
    ahead=theAhead;
    back=theBack;
  }  
   
  public Baggage getItem(){return bagItem;}
  public BagNode getAhead(){return ahead;}
  public BagNode getBack(){return back;}
  public void setAhead(Node newAhead){ahead=newAhead;}
  public void setBack(Node newBack){back=newBack;}
}//end class BagNode

class ConveyorBelt{
  private static int count=0;
  private static int countVIP=0;
  private static double totalWeight=0;
  private BagNode top;
  private BagNode end;
  
  public static int readCount(){
    return count;
  }
  
  public ConveyorBelt(){ 
    top=null;
    end=null;
  }
  
  public void addVIP(Baggage bag){
    
  }
  
  public void addToEnd(Baggage bag){
     IntNode newNode=new IntNode(data,null);
    IntNode current=top;
    if(top==null){
      top=newNode;
      return;
    }  
    while(current.getLink()!=null){
      current=current.getLink();
    }
    current.setLink(newNode);
  }
  
  public void printSummary(){
    System.out println("Total number of bags: "+count+", Number of VIP bags: "+countVIP+", Total weight of bags: "+totalWeight+".");
  }
  
  public void printDetail(){
    printSummary();
    System.out println("The bags on the conveyor belt are:");
    to String
  }
  
  public int loadFlight(int flight){
  }
    
  public int removeOversize(double size){
  }
}//end class ConveyorBelt

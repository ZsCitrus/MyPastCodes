import java.util.*;
import java.io.*;
/*
* A3ZhouXing.java
* COMP 2140 SECTION A01
* INSTRUCTOR Cameron (A01)
* ASSIGNMENT Assignment 3
* Name:Xing Zhou
* Student Number:7869781
* Compeletion date: Nov 12
* 
* PURPOSE: use stack and queue to model an elevator
*/

public class A3ZhouXing{
    
  public static void main(String[] args)throws Exception{
    processInputFile();
    System.out.println("Processing ends normally.");
  }//end main function

  public static void processInputFile() throws Exception{
    System.out.println("\nEnter the input file name (txt files only): ");  
    Scanner keyboard=new Scanner(System.in);  
    String fileName=keyboard.nextLine();  
    try{
      FileReader fileInput=new FileReader(fileName);
      BufferedReader line=new BufferedReader(fileInput);
      String lineContent=line.readLine();
      String[] elevatorProperty=lineContent.split(" "); //process the first line for the property of the elevator
      int capacity=Integer.parseInt(elevatorProperty[0]);
      int floor=Integer.parseInt(elevatorProperty[1]);
      Elevator elevator=new Elevator(capacity, floor);
      System.out.println("Elevator dimensions:");
      System.out.println("Number of floors to service: "+floor);
      System.out.println("Maximum number of people(load capacity): "+capacity);
      System.out.println("Elevator begins on floor "+elevator.getFloor());
      lineContent=line.readLine(); //process the employee information
      while(lineContent!=null){
        String[] employeeData=lineContent.split(" ");
        int timeArrive=Integer.parseInt(employeeData[0]);
        int employeeID=Integer.parseInt(employeeData[1]);
        int floorOn=Integer.parseInt(employeeData[2]);
        int floorOff=Integer.parseInt(employeeData[3]);
        if(timeArrive==elevator.getTime()){ //enter the queue when time matches
          Employee e=new Employee(timeArrive, employeeID, floorOn, floorOff);
          elevator.employeeArrival(e);
          if(line!=null) //in case the input file does not have a new line char at the last line
            lineContent=line.readLine();
          else
            lineContent=null;
        }
        else{ //otherwise just elevator take actions
          elevator.takeActions();
          elevator.clearOperationCount();
          if(elevator.finish()){ //in case the employee arrival time has large gap
            elevator.setTime(timeArrive); //make the input file reading process can continue
          }                               //by letting the elevator time jump across the gap
        } 
      }
      line.close();
      fileInput.close(); //finish reading input file
      while(!elevator.finish()){ // while the simulation process has not finished
        elevator.takeActions();
        elevator.clearOperationCount();
      }
      elevator.printStatistics(); //after finish, print statistics
    }
    catch(IOException e){
      System.out.println("enconter problems when reading file");
    }
  }//end processInputFile function
}//end main(A3ZhouXing) class

class Elevator{
  private static final int WAITING_LINES=2;
  private static final String[]DIRECTIONS={"up","down"};
  private int maxCapacity;
  private int totalFloor;
  private Stack employeeInElevator;
  private Queue[][] employeeWaitElevator;
  private int currentFloor;
  private String currentDirection;
  private int elevatorTime;
  //to control the elevator time change during each call of the take action function, details see below in the function
  private int operationCount; 
  private int trips;
  private int passengerTimes;
  private int minTime;
  private int maxTime;
  private Employee min;
  private Employee max;
  
  public Elevator(int c, int f){
    maxCapacity=c;
    totalFloor=f;
    employeeInElevator=new Stack(c);
    employeeWaitElevator=new Queue[f][WAITING_LINES];
    for(int i=0; i<f; i++){
      for(int j=0;j<WAITING_LINES; j++){
        employeeWaitElevator[i][j]=new Queue();
      }
    }
    currentFloor=0;
    currentDirection=DIRECTIONS[0];
    elevatorTime=0;
    operationCount=0;
    trips=0;
    passengerTimes=0;
    minTime=Integer.MAX_VALUE;
    maxTime=Integer.MIN_VALUE;
    min=null;
    max=null;
  }
  
  public int getFloor(){
    return currentFloor;
  }
  
  public int getTime(){
    return elevatorTime;
  }
  
  public void clearOperationCount(){ //detailed comments in take action function
    operationCount=0;
  }
   
  public void setTime(int t){
    elevatorTime=t;
  }
  
  //model the elevator behavior
  public void takeActions(){
    if(currentDirection.equals(DIRECTIONS[0])){ //at first decide whether change the direction
      if((currentFloor>=totalFloor-1)||
         (employeeInElevator.isEmpty()&&emptyInCurrentDirection()&&!emptyInOppositeDirection())){
        currentDirection=DIRECTIONS[1];
        System.out.println("Time "+elevatorTime+": Elevator changed direction: Now going "+currentDirection+".");
      }
      //the operation count variable will switch between 0 and 1 to make sure that every time the action function
      //is called, the elevator time will only increase one unit.
      //otherwise elevator time can increase several units after calling the function, thus make the elevator time
      //will never match the input file arrival time, then the input reading process will be destroyed.
      if(operationCount<1){ //decide whether open door to let on and off
        boolean off=someoneGetoff(); //do the getting off or on process the same time with do the judgement
        boolean on=someoneGeton();
        if(off||on){
          elevatorTime++;
          operationCount++;
        }
      } 
      if((operationCount<1)&&(!employeeInElevator.isEmpty()||!emptyInCurrentDirection())){
        move(); //move to next floor or do nothing
        elevatorTime++;
        operationCount++;
      }
    }
          
    if(currentDirection.equals(DIRECTIONS[1])){
      if((currentFloor<=0)||
         (employeeInElevator.isEmpty()&&emptyInCurrentDirection()&&!emptyInOppositeDirection())){
        currentDirection=DIRECTIONS[0];
        System.out.println("Time "+elevatorTime+": Elevator changed direction: Now going "+currentDirection+".");
      }
      if(operationCount<1){
        boolean off=someoneGetoff();
        boolean on=someoneGeton();
        if(off||on){
          elevatorTime++;
          operationCount++;
        }
      }
      if((operationCount<1)&&(!employeeInElevator.isEmpty()||!emptyInCurrentDirection())){
        move();
        elevatorTime++;
        operationCount++;
      }
    }      
  }

  //check whether anyone waiting in the current direction
  private boolean emptyInCurrentDirection(){
    boolean empty=true;
    if(currentDirection.equals(DIRECTIONS[0])){
      for(int i=currentFloor; i<totalFloor; i++){
        for(int j=0;j<WAITING_LINES; j++){
          if(!employeeWaitElevator[i][j].isEmpty())
            empty=false;  
        }
      }
    }   
    if(currentDirection.equals(DIRECTIONS[1])){
      for(int i=currentFloor-1; i>=0; i--){
        for(int j=0;j<WAITING_LINES; j++){
          if(!employeeWaitElevator[i][j].isEmpty())
            empty=false;
        }
      }
    }
    return empty;
  }

  //check whether anyone waiting in the opposite direction
  private boolean emptyInOppositeDirection(){
    boolean empty=true;
    if(currentDirection.equals(DIRECTIONS[0])){
      for(int i=currentFloor-1; i>=0; i--){
        for(int j=0;j<WAITING_LINES; j++){
          if(!employeeWaitElevator[i][j].isEmpty())
            empty=false;
        }
      }
    }   
    if(currentDirection.equals(DIRECTIONS[1])){
      for(int i=currentFloor; i<totalFloor; i++){
        for(int j=0;j<WAITING_LINES; j++){
          if(!employeeWaitElevator[i][j].isEmpty())
            empty=false;
        }
      }
    }
    return empty;
  }

  //do the getting off, and do the judagment at the same time
  private boolean someoneGetoff(){
    boolean getoff=false;
    Stack temp=new Stack(maxCapacity);
    int stackCount=employeeInElevator.getCount();
    int tempCount=0;
    while(stackCount>0){
      if(employeeInElevator.top().getFloorLeave()==currentFloor){
         getoff=true;
         Employee leave=employeeInElevator.pop();
         System.out.println("Time "+elevatorTime+": Got off the elevator: "+leave.toString());
         int times=elevatorTime-leave.getTimeArrive(); //calculate the statistics
         passengerTimes+=times;
         if(minTime>times){
           minTime=times;
           min=leave;
         }
         if(maxTime<times){
           maxTime=times;
           max=leave;
         }
      }
      else{ //use temporary stack to hold the people who will not get off at the current floor
        temp.push(employeeInElevator.pop());
        tempCount++;
      }
      stackCount--;
    }
    while(tempCount>0){
      employeeInElevator.push(temp.pop());
      tempCount--;
    }
    return getoff;
  }
  
  //do the getting on, and do the judagment at the same time
  private boolean someoneGeton(){
    boolean geton=false;
    if(currentDirection.equals(DIRECTIONS[0])){  
      while(employeeInElevator.getCount()<maxCapacity&&!employeeWaitElevator[currentFloor][0].isEmpty()){
        geton=true;
        employeeInElevator.push(employeeWaitElevator[currentFloor][0].leave());
        System.out.println("Time "+elevatorTime+": Got on the elevator: "+employeeInElevator.top().toString());
      }
    }
    if(currentDirection.equals(DIRECTIONS[1])){
      while(employeeInElevator.getCount()<maxCapacity&&!employeeWaitElevator[currentFloor][1].isEmpty()){
        geton=true;
        employeeInElevator.push(employeeWaitElevator[currentFloor][1].leave());
        System.out.println("Time "+elevatorTime+": Got on the elevator: "+employeeInElevator.top().toString());
      }
    }
    return geton;
  }

  //move to next floor
  private void move(){
    if(currentDirection.equals(DIRECTIONS[0])){
       currentFloor++;
       System.out.println("Time "+elevatorTime+": Elevator moves "+currentDirection+" to floor "+currentFloor);
    }
    if(currentDirection.equals(DIRECTIONS[1])){
       currentFloor--;
       System.out.println("Time "+elevatorTime+": Elevator moves "+currentDirection+" to floor "+currentFloor);
    }
  }
  
  //process employee arriving
  public void employeeArrival(Employee e){
    if(e.getDirection().equals(DIRECTIONS[0])){
      employeeWaitElevator[e.getFloorArrive()][0].enter(e);
      System.out.println("Time "+elevatorTime+": A person begins waiting to go "+e.getDirection()+": "+e.toString());
    }
    if(e.getDirection().equals(DIRECTIONS[1])){
      employeeWaitElevator[e.getFloorArrive()][1].enter(e);
      System.out.println("Time "+elevatorTime+": A person begins waiting to go "+e.getDirection()+": "+e.toString());
    }
    trips++; //calculate statistics
  }
  
  //help to finish running the simulation after the last employee arrival
  public boolean finish(){
    return employeeInElevator.isEmpty()&&emptyInCurrentDirection()&&emptyInOppositeDirection();
  }
  
  //print statistics
  public void printStatistics(){
    System.out.println("Elevator simulation statistics:");
    System.out.println("Total number of trips: "+trips);
    System.out.println("Total passenger time: "+passengerTimes);
    double average=(double)passengerTimes/(double)trips;
    System.out.printf("Average trip time:%6.2f%n",average);
    System.out.println("Minimum trip time: "+minTime);
    System.out.println("Minimum trip details: "+min.toString());
    System.out.println("Maximum trip time: "+maxTime);
    System.out.println("Maximum trip details: "+max.toString());
  }
}

//stack implemented by array
class Stack{
  private Employee[] employeeStack;
  private int count;
  
  public Stack(int max){
    employeeStack=new Employee[max];
    count=0;
  }
  
  public boolean isEmpty(){
    return count==0;
  }
  
  public void push(Employee e){
    employeeStack[count]=e;
    count++;
  }
  
  public Employee top(){
    return employeeStack[count-1];
  }
  
   public Employee pop(){
     Employee e=employeeStack[count-1];
     employeeStack[count-1]=null;
     count--;
     return e;
  }
   
   public int getCount(){
     return count;
   }
}

//queue implemented by circular linked list
class Queue{
  private Node end;
  
  public Queue(){
    end=null;
  }
  
  public boolean isEmpty(){
    return end==null;
  }
  
  public void enter(Employee e){
    if(end==null){
      end=new Node(e,null);
      end.next=end;
    }
    else{
      end.next=new Node(e,end.next);
      end=end.next;
    }
  }
  
  public Employee leave(){
    Employee e=null;
    if(end.next==end){
      e=end.next.item;
      end=null;
    }
    else{
      e=end.next.item;
      end.next=end.next.next;
    }
    return e;
  }
    
  private class Node{
    public Employee item;
    public Node next;
  
    public Node(Employee e, Node n){
      item=e;
      next=n;
    }
  }
}

//employee class
class Employee{
  private static final String[]DIRECTIONS={"up","down"};
  private int timeArrive;
  private int ID;
  private int floorArrive;
  private int floorLeave;
  
  public Employee(int time, int id, int on, int off){
    timeArrive=time;
    ID=id;
    floorArrive=on;
    floorLeave=off;
  }
  
  public int getTimeArrive(){
    return timeArrive;
  }
  
  public int getFloorArrive(){
    return floorArrive;
  }
  
  public int getFloorLeave(){
    return floorLeave;
  }
  
  public String getDirection(){
    String direction=null;
    if(floorArrive<floorLeave)
      direction=DIRECTIONS[0];
    if(floorArrive>floorLeave)
      direction=DIRECTIONS[1];
    return direction;
  }
  
  public String toString(){
    return "Employee "+ID+", arrival floor "+floorArrive+", arrival time "+timeArrive+", desired floor "+floorLeave;
  }
}
import java.io.*;
import java.util.*;

public class CS3380A3P1{
  private static final String FILE1="people.txt"; //here to change input files
  private static final String FILE2="activities.txt";
  private static ArrayList<String> table=new ArrayList<String>();
  private static ArrayList<String> append=new ArrayList<String>();
  private static int outer=0;
  public static void main(String[] args){
    leftJoin(FILE1, FILE2);
    System.out.println("---------inner join---------");
    printTable();
    System.out.println("---------left join---------");
    printTable();
    printLeftAppend();
    System.out.println("---------full outer join---------");
    printTable();
    printLeftAppend();
    fullOuterJoin(FILE1, FILE2);
    printRightAppend();
  }
  
  public static void leftJoin(String file1, String file2){ 
    int count=0;
    BufferedReader in1=null;
    BufferedReader in2=null;
    try{
      in1=new BufferedReader((new FileReader(file1)));
      in1.readLine();
      String line1=in1.readLine();
      while(line1!=null){
        count=0;
        String parse1=line1;
        String[] part1=parse1.split(",");
        in2=new BufferedReader((new FileReader(file2)));
        in2.readLine();
        String line2=in2.readLine();
        while(line2!=null){  
          String parse2=line2;
          String[] part2=parse2.split(",");
          outer=part2.length;
          if(part2[0].equals(part1[0])){
            String tuple=line1+","+line2;
            table.add(tuple);
            count++;
          }
          line2=in2.readLine();
        }
        if(count==0){
          append.add(line1);
        }
        line1=in1.readLine();
      }
      in1.close();
      in2.close();
    }
    catch (IOException e) {
      e.printStackTrace();
    } 
  }
  
  public static void fullOuterJoin(String file1, String file2){
    table.clear();
    append.clear();
    outer=0;
    leftJoin(file2, file1);
  }
  
  public static void printTable(){
    for(int i=0; i<table.size(); i++){
      System.out.println(table.get(i));
    }
  }
  
  public static void printLeftAppend(){
    for(int i=0; i<append.size(); i++){
      String addLine=append.get(i);
      for(int j=0; j<outer; j++){
        addLine+=",Null";
      }
      System.out.println(addLine);
    }
  }
  
  public static void printRightAppend(){
    for(int i=0; i<append.size(); i++){
      String addLine="";
      for(int j=0; j<outer; j++){
        addLine+="Null,";
      }
      addLine=addLine+append.get(i);
      System.out.println(addLine);
    }
  }
}
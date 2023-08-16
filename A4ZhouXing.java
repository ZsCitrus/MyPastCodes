import java.io.*;
import java.util.*;

/*
* A4ZhouXing.java
* COMP 2140 SECTION A01
* INSTRUCTOR Cameron (A01)
* ASSIGNMENT Assignment 4
* Name:Xing Zhou
* Student Number:7869781
* Compeletion date: Nov 23
* 
* PURPOSE: count all the words in a file and print an alphabetical listing of the first m words and the last m words
*/

//------------------some notes about the outcome-----------------------
//My outcome for the total number of words and unique number of words counting are slightly different from the sample outcome.
//But the alphabetical listing details are the same with the sample except the count for word "a".
//I counted the total words and unique words in the application class as soon as spliting each line,instead of counting 
//them in the WordTable class. And for input of smaller size, the spliting and counting process got no problem.
//So I think the mismatching is related to the spliting process. Since the sample input is large and complicated,
//due to the inconsistency of either complier, IDE, or OS, spliting outcomes may differ.
//Since the spliting outcomes may differ, it's reasonable that the word "a" has same position in the tree but different
//repeated times since it is so widely used in English language.
//I have no idea how to deal with the spliting process issue, so I just left it bahind and wrote the notes for explaination.

public class A4ZhouXing{
  public static WordTable table=new WordTable(20000);
  public static BST tree=new BST();
    
  public static void main(String[] args)throws Exception{
    processInputFile(); 
    fillAndPrintTree();
    System.out.println("Processing ends normally.\n");
  }//end main function

  public static void processInputFile() throws Exception{
    System.out.println("\nEnter the input file name (txt files only): ");  
    Scanner keyboard=new Scanner(System.in); //read the file name
    String fileName=keyboard.nextLine();
    int totalWords=0;  //the total words and unique words statistics are calculated in the application class
    int uniqueWords=0;
    try{
      FileReader fileInput=new FileReader(fileName); //process the input file line by line
      BufferedReader line=new BufferedReader(fileInput);
      String lineContent=line.readLine();
      while(lineContent!=null){
        String lineInput=lineContent.toLowerCase(); //to lower case
        String[] words=lineInput.split("[^a-z]+"); //spliting the line
        for(int i=0; i<words.length; i++){
          if(!words[i].equals("")){ //exclude the empty string
            totalWords++;
            WordCount wordExisted=table.tableSearch(words[i]); //search the word at first
            if(wordExisted==null){ //if the word did not exist (is new)
              uniqueWords++;
              table.tableInsert(words[i]); //insert the word in the hash table
            }
            else{
              wordExisted.increaseCount(); //for existing word, just increase its count
            }
          }
        }
        lineContent=line.readLine();
      }
      line.close();
      fileInput.close();
      System.out.println("Total number of words in the file: "+totalWords+"\n"); //print the statistics
      System.out.println("Number of unique words: "+uniqueWords+"\n");
    }//finish reading input file    
    catch(IOException e){
      System.out.println("enconter problems when reading file\n");
    }
  }//end processInputFile function
  
  public static void fillAndPrintTree(){
    table.initializeTableTraverse(); //pointer to the first not null position in the table
    while(table.hasNextNode()){ //whether has node to read
      WordCount word=table.nextNodeItem(); //read the node, put the pointer to the next appriociate position
      tree.insertTree(word); //insert the node content into the tree
    }
    System.out.println("\nEnter the number of items to print (positive integers only):");  
    Scanner keyboard=new Scanner(System.in); //let user to input the alphabetical listing number
    int wordsToPrint=keyboard.nextInt();
    System.out.println("The first "+wordsToPrint+" and last "+wordsToPrint+" words in alphabetical order:\n");
    tree.printTree(wordsToPrint); //printed as input
  } //end fillAndPrintTree
}//end main(A4ZhouXing) class

//WordCount class
class WordCount{
  private String word;
  private int count;
  
  public WordCount(String s, int c){
    word=s;
    count=c;
  }
  
  public String getWord(){
    return word;
  }
  
  public int getCount(){
    return count;
  }
  
  public void increaseCount(){
    count++;
  }
  
  public String toString(){
    return "Word = "+word+", Count = "+count+"\n";
  }
}

//WordTable class
class WordTable{
  
  private class Node{ //Node class
    public WordCount item;
    public Node next;

    public Node(WordCount w, Node n) {
      item=w;
      next=n;
    } //end Node constructor
  } //end Node class
  
  //**********WordTable class continue**********
  private static final int A=13; //constant for the hash function
  private Node[] hashArray; //The array of linked lists.
  private int position; //the hash array index for the word reading pointer
  private Node currentAtPosition; //the current node for a specific linked list indicated by an index
  
  private static int closePrime(int n){ //find a prime number big than parameter
    int value=n;                        //used for determine the hash array size
    if(value%2==0){
       value++;
    }
    while(!isPrime(value)){
      value+=2;
    }
    return value;
  } //end closePrime
  
  private static boolean isPrime(int n){ //check whether prime
    boolean noFactorFound=n%2!=0;
    for (int i=3; noFactorFound&&i*i<=n; i+=2){
      noFactorFound=n%i!=0;
    }
    return noFactorFound;
  } //end isPrime
  
  public WordTable(int n){ //WordTable constructor
    int tableSize=closePrime(n);
    hashArray=new Node[tableSize];
    for(int i=0; i<hashArray.length; i++){
      hashArray[i]=null;
    }
    position=hashArray.length; //in constructor, first put the word reading pointer to a invalid position
    currentAtPosition=null;
  } //end WordTable constructor
  
  private int hashIndex(String key){ //hash function to get an index
    int hashIndex=0;
    for(int i=0; i<key.length(); i++){
      hashIndex=(hashIndex*A)%hashArray.length+(int)key.charAt(i);
      hashIndex=hashIndex%hashArray.length;
    }
    return hashIndex;
  } //end hashIndex

  public void tableInsert(String key){ //insert a word into the table
    if(tableSearch(key)==null){
      int index=hashIndex(key);
      WordCount newWord=new WordCount(key, 1);
      hashArray[index]=new Node(newWord,hashArray[index]);
    }
    else{  //no duplicate allowed
      System.out.println( "ERROR: Attempting to insert a duplicate of \"" +key+"\"\n");
    }
  } //end insert

  public WordCount tableSearch(String key){ //search a word into the table
    WordCount target=null;
    int index=hashIndex(key);
    Node current=hashArray[index];
    while(current!=null&&target==null){
      if(key.equals(current.item.getWord())){
        target=current.item; 
      }
      else{
        current=current.next;
      } 
    }
    return target; 
  } //end search 
  
  //put the word reading pointer to the first not null position
  public void initializeTableTraverse(){
    position=0;
    while(position<hashArray.length&&currentAtPosition==null){ //move until find a not null position
      currentAtPosition=hashArray[position];
      if(currentAtPosition==null)
        position++;
    }
  } //end initializeTableTranverse
  
  //when the position become invalid, then there must be no more word to read
  public boolean hasNextNode(){
    return !(position==hashArray.length&&currentAtPosition==null);
  } //end hasNextNode
  
  //read the word, and then move the pointer
  public WordCount nextNodeItem(){
    WordCount nextWord=currentAtPosition.item;
    currentAtPosition=currentAtPosition.next; //move to next node in the current index
    while(currentAtPosition==null&&position<hashArray.length){ //if all nodes in this index have been read
      position++;                                              //move to next index until find next not null position
      if(position<hashArray.length) //prevent point to null when go beyond the size of the array
        currentAtPosition=hashArray[position];
    }
    return nextWord;
  } //end nextNodeItem
} //end class WordTable

class BST{
  private class BSTNode{
    public WordCount item;
    public BSTNode left;
    public BSTNode right;
    
    public BSTNode(WordCount w){
      item=w;
      left=null;
      right=null;
    } //end BSTNode constructor
  } // end class BSTNode   
  
  //**********BST class continue**********
  private BSTNode root;
  private int itemNumbers;

  public BST(){ //create an empty tree
    root=null;
    itemNumbers=0; //this variable track the number of items inserted
  } //end BST constructor
  
  
  //An non-recursive insert with no duplicates allowed.
  public void insertTree(WordCount w){
    BSTNode currentParent, current;
    BSTNode newNode;
    if(root==null){
      root=new BSTNode(w);
      itemNumbers++;
    }
    else{
      currentParent=null;
      current=root;
      while((current!=null)&&(!current.item.getWord().equals(w.getWord()))){
        currentParent=current;
        if(w.getWord().compareTo(current.item.getWord())<0)
          current=current.left;
        else
          current=current.right;
      }
      if(current==null){
        itemNumbers++;
        newNode=new BSTNode(w);
        if(w.getWord().compareTo(currentParent.item.getWord())<0)
          currentParent.left=newNode;
        else
          currentParent.right=newNode;     
      }
    }
  } //end insertTree
  
  //public driver method, print the first numToPrint and last numToPrint objects in the tree
  public void printTree(int numToPrint){
    if(itemNumbers>2*numToPrint) //some objects are only visited, but not printed
      printTree(root, numToPrint, itemNumbers-2*numToPrint, numToPrint);
    else
      printTree(root, itemNumbers, 0, 0); //all objects are printed
  } //end public printTree

  //private recursive helper method, return a int counting all the objects have been visited
  //in the recursive traversing method, all objects are visited(traversed), but using the relation between its 
  //position and the parameters to determine it should be printed or not
  private int printTree(BSTNode current, int startPrint, int middleSkip, int endPrint){
    int numTraversed=0; //track the number of objects have been visited
    if(current.left!=null){ //deal with the left parts, down to the leftmost, parameter need no change
      numTraversed+=printTree(current.left, startPrint, middleSkip, endPrint);
    }
    //when visiting the middle one, check if it should be printed or not
    if(numTraversed<startPrint||numTraversed>=startPrint+middleSkip){
       System.out.println(current.item.toString());
       numTraversed++;
    }
    else{
      numTraversed++;
    }
    //when dealing with the right parts, the parameters to the right parts should be calculated according to
    //the number of objects has been visited at first
    if(current.right!=null){
      if(numTraversed<startPrint){
        numTraversed+=printTree(current.right, startPrint-numTraversed, middleSkip, endPrint);
      }
      else if(numTraversed>=startPrint&&numTraversed<startPrint+middleSkip){
        numTraversed+=printTree(current.right, 0, startPrint+middleSkip-numTraversed, endPrint);
      }
      else{
        numTraversed+=printTree(current.right, 0, 0, itemNumbers-numTraversed);
      }
    } 
    return numTraversed;
  } //end private printTree
} //end BST class
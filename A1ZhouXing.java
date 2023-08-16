/*
* A1ZhouXing.java
* COMP 2140 SECTION A01
* INSTRUCTOR Cameron (A01)
* ASSIGNMENT Assignment 1
* Name:Xing Zhou
* Student Number:7869781
* Compeletion date:Oct 12
* 
* PURPOSE: compare the time of execution of 5 sorting algorithms
*/

public class A1ZhouXing{
  
  //the size of the array to be sorted, swap times and sort times are consistent according to the requirement
  //thus use constant to control them
  private static final int ITEMS=10000;
  private static final int SWAPS=5000;
  private static final int TIMES=100;
  
  //repeating codes in the test method can be avoided by using these constant to control which algorithm should
  //be used each time trying to sort, more details see the method below
  private static final int ALGONUM=5;//total number of algorithms
  private static final String ALGORITHMS[]={"insertionSort","selectionSort","mergeSort","quickSort","hybridQuickSort"};
  
  //the breakpoint list required by the discription of the assignment
  private static final int[] BREAKPOINT_SET={25,35,45,55,65,75,85,95,100};
  
  //the global variable breakpoint
  private static int breakpoint;
  
  public static void main(String[] args){
    System.out.println("The average excution time of the 5 sorting algorithms are shown below:");
    test();
    System.out.println("This is the end of the program.");
  } 
  
  public static void insertionSort(int[]array){
    insertionSort(array,0,array.length);
  }
  
  //the insertion sort which can sort part of an array
  private static void insertionSort(int[]array, int start, int end){
    int temp;//store the value to be insert
    int j;
    for(int i=start+1; i<end; i++){
       temp=array[i];
       j=i-1;//search start at the end of the sorted part
       while(j>=start && array[j]>temp){
         array[j+1]=array[j];
         j--;//go from right to left
       }
      array[j+1]=temp;
    }
  }
  
  public static void selectionSort(int[] array){
    for(int i=0;i<array.length;i++){
      int j=findMin(array, i, array.length);
      int temp=array[i];
      array[i]=array[j];
      array[j]=temp;
    }
  }
  
  private static int findMin(int[] array, int start, int end){
    int index=start;
    int min=array[start];
    for(int i=start;i<end;i++){
      if(array[i]<min){
        min=array[i];
        index=i;
      }
    }
    return index;
  }
  
  //the double version of find min, to be used in the choose breakpoint method to determine the fastet breakpoint
  private static int findMin(double[] array, int start, int end){
    int index=start;
    double min=array[start];
    for(int i=start;i<end;i++){
      if(array[i]<min){
        min=array[i];
        index=i;
      }
    }
    return index;
  }
  
  public static void mergeSort(int[] array){
      int[] temp = new int[array.length];
      mergeSort(array, 0, array.length, temp);
  }
  
   private static void mergeSort(int[] array, int start, int end, int[] temp){
      int mid;
      if(end-start>2){ //recursion
        mid=start+(end-start)/2;
        mergeSort(array, start, mid, temp);
        mergeSort(array, mid, end, temp);
        merge(array, start, mid, end, temp);
      }
      else if(end-start==2){ //base case
        if(array[end-1]<array[start]){
          int swap=array[start];
          array[start]=array[end-1];
          array[end-1]=swap;
        }
      }
     //base case: end-start<2, only 1 or 0 item in the sublist, do nothing.
   }
   
   private static void merge(int[] array, int start, int mid, int end, int[] temp){
     int currL=start; //index of current item in left half
     int currR=mid; //index of current item in right half
     int currT; //index in temp
     for(currT=start; currT<end; currT++){
       if(currL<mid && (currR>=end||array[currL]<array[currR])){
       //copy from left half if that value is smaller or if no values remain in the right half
          temp[currT]=array[currL];
          currL++;
        }
       else{ //copy from the right half
         temp[currT]=array[currR];
         currR++;
       }
     }
     for(currT=start; currT<end; currT++){
       array[currT]=temp[currT];//copy the sorted sublist back to original array
     }
   }
   
   public static void quickSort(int[] array){
    quickSort(array, 0, array.length);
   }
  
   private static void quickSort(int[] array, int start, int end){
     if(end-start>2){ //recursion
       choosePivot(array, start, end);
       int pivotIndex=partition(array, start, end);
       quickSort(array,start,pivotIndex);
       quickSort(array,pivotIndex+1,end); //do not touch the pivot
     }
     else if(end-start==2){ //base case
       if(array[end-1]<array[start]){
          int swap=array[start];
          array[start]=array[end-1];
          array[end-1]=swap;
       }
     }
     //base case: end-start<2, only 1 or 0 item in the sublist, do nothing.
   }
  
  private static void choosePivot(int[] array, int start, int end){
    int mid=start+(end-start)/2;
    int medianIndex=-1;//no new array are permitted, cannot use findMin, thus this a feasible way to find the median
    if((array[start]>=array[mid]&&array[start]<=array[end-1])||(array[start]>=array[end-1]&&array[start]<=array[mid]))
      medianIndex=start;
    if((array[mid]>=array[start]&&array[mid]<=array[end-1])||(array[mid]>=array[end-1]&&array[mid]<=array[start]))
      medianIndex=mid;
    if((array[end-1]>=array[mid]&&array[end-1]<=array[start])||(array[end-1]>=array[start]&&array[end-1]<=array[mid]))
      medianIndex=end-1;
    if(array[start]!=array[medianIndex]){ //swap the median and the start position as needed
      int temp=array[start];
      array[start]=array[medianIndex];
      array[medianIndex]=temp;
    }
  }
  
  private static int partition(int[] array, int start, int end){
    int bigStart=start+1;
    for(int current=start+1;current<end;current++){
      if(array[current]<array[start]){ //if the currently processed item belongs to smalls, swap with position bigStart
        int temp=array[bigStart];
        array[bigStart]=array[current];
        array[current]=temp;
        bigStart++;
      } //otherwise do nothing
    }
    int temp=array[start];// put the pivot into the right position
    array[start]=array[bigStart-1];
    array[bigStart-1]=temp;
    return bigStart-1;
  }
  
  public static void hybridQuickSort(int[] array){
    hybridQuickSort(array, 0, array.length);
  }
  
  private static void hybridQuickSort(int[] array, int start, int end){
    if(end-start>=breakpoint){ //recursion
      choosePivot(array, start, end);
      int pivotIndex=partition(array, start, end);
      hybridQuickSort(array,start,pivotIndex);
      hybridQuickSort(array,pivotIndex+1,end);//do not touch the pivot
    }
    else{
      insertionSort(array, start, end);
    }
  } 
  
  //test whether a list sorted or not
  public static boolean isSorted(int[] array){
    boolean sorted=true;
    for(int i=0;i<array.length-1;i++){
      if(array[i]>array[i+1])
      sorted=false;
    }
    return sorted;
  }
  
  public static void fillArray(int[] array){
    for(int i=0;i<array.length;i++){
      array[i]=i;
    }
  }
  
  public static void randomizeArray(int[] array, int n){
    for(int i=0;i<n;i++){ //do n swaps
       int indexA=randomInt(array.length);
       int indexB=randomInt(array.length);
       int temp=array[indexB];
       array[indexB]=array[indexA];
       array[indexA]=temp;
    }
  }
  
  //help method to generate a random integer index between 0 and array length
  private static int randomInt(int a){
    double number=a*Math.random();
    int integer=(int)number;
    return integer;
  }
  
  public static void test(){
    breakpoint=chooseBreakpoint();//determine a breakpoint at first
    long[] timeSet=new long[TIMES];//array to store the 100 sorting time value
    int[] array=new int[ITEMS];//size of array to be sorted is 10000
    fillArray(array);
    long startTime, stopTime;//system time record
    for(int i=0;i<ALGONUM;i++){ //use the sorting algorithms one by one
      for(int j=0; j<timeSet.length; j++){ //for each algorithms, sort 100 times
        randomizeArray(array,SWAPS);
        if(isSorted(array)) //make sure the array is randomized
          System.out.println(ALGORITHMS[i]+" randomize round "+j+" failed.");
        //use this way to control which method to be used can effectively avoid repeating codes
        //although repeating codes are permitted, this make the test method much more clear
        startTime=System.nanoTime();
        if(i==0)
          insertionSort(array);
        if(i==1)
          selectionSort(array);
        if(i==2)
          mergeSort(array);
        if(i==3)
          quickSort(array);
        if(i==4)
          hybridQuickSort(array);
        stopTime=System.nanoTime();
        if(!isSorted(array)) //make sure the array is sorted after calling a sorting method
          System.out.println(ALGORITHMS[i]+" sort round "+j+" failed.");
        timeSet[j]=stopTime-startTime; //store the time value in the array
      }
    //calculate and print the outcome
    System.out.println("arithmetic mean of "+ALGORITHMS[i]+" is "+arithmeticMean(timeSet));
    }
  }
  
  public static int chooseBreakpoint(){
    long[] timeSet=new long[TIMES]; //array to store the 100 sorting time value
    //array to store the one by one arithmetic mean value for every breakpoint
    //thus the findMin method can be used to find min
    double[] breakpointMeanTime=new double[BREAKPOINT_SET.length];
    int[] array=new int[ITEMS];//size of array to be sorted is 10000
    fillArray(array);
    long startTime, stopTime;//system time record
    for(int i=0; i<BREAKPOINT_SET.length; i++){ //use the pre-determind breakpoint one by one
      breakpoint=BREAKPOINT_SET[i];
      for(int j=0; j<timeSet.length; j++){ //for every breakpoint, sort 100 times
        randomizeArray(array,SWAPS);
        if(isSorted(array)) //make sure the array is randomized
          System.out.println("Breakpoint "+breakpoint+" randomize "+j+" failed.");
        startTime=System.nanoTime();
        hybridQuickSort(array);
        stopTime=System.nanoTime();
        if(!isSorted(array)) //make sure the array is sorted after calling hybrid quick sorting
          System.out.println("Breakpoint "+breakpoint+" sort "+j+" failed.");
        timeSet[j]=stopTime-startTime;//store the time value in the array
      }
      breakpointMeanTime[i]=arithmeticMean(timeSet);//calculate the arithmetic mean
    }
    //find the index of the min arithmetic mean, use this index to find the associated breakpoint
    int best=BREAKPOINT_SET[findMin(breakpointMeanTime,0,breakpointMeanTime.length)];
    return best;
  }

  //Compute the arithmetic mean of long values,to avoid long overflow, use type double in the computation.
  public static double arithmeticMean(long data[]){
    double sum=0;
    for(int i=0; i<data.length; i++)
      sum+=(double)data[i];
    return sum/(double)data.length;
  }
} //end of class

/* Here is the small report.
   1. Was insertion sort faster than selection sort? Why or why not?
   Yes, insertion sort is faster than selection sort.
   Because for every scenario in the n depth of selection sort, the comparision process will thoroughly go through
   the unsorted part with no shortcut.
   But for every scenario in the n depth of insertion sort, the insertion process may need not to go thoroughly through
   the sorted part, thus saving some time.
   
   2. Was quick sort faster than insertion sort? Why or why not?
   Yes, quick sort is faster than insertion sort.
   Because the excuting time for quick sort is O(nlogn), while for insertion sort is O(n2).
   When n is big enough, nlogn definitely beats n2.
   
   3. Was hybrid quick sort faster than the quick sort? Why or why not?
   Yes, hybrid quick sort is faster than quick sort.
   When n is small enough, nlogn won't beat n2, so under such situation insertion sort can be faster than quick sort.
   This assertion can be tested by changing the constants at the top of the program.
   Thus when the list is broken down to sublists which are small enough, sorting the sublists by insertion sort 
   is faster than quick sort.
   This is exactly what hybrid quick sort does, so it is faster.
   
   4. Which sort would you recommend to others, and why would you recommend that one?
   I will recommend hybrid quick sort since it is the fastest.
   
   5. Which sort would you warn others against using, and why?
   I will warn the selection sort since it is the slowest.
*/
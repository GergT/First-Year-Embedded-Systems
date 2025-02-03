
// Up arrow custom character
byte upChar[] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

// Down arrow custom character
byte downChar[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};


#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <MemoryFree.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Each employee's info is stored in a struct as shown below.
struct Employee {
  String ID;
  String jobTitle;
  String pension;
  int grade = -1;
  float salary;
};

// Determines the maximum amount of employees you are going to store on the system that can be changed with ease.
const int EMPLOYEECOUNT = 30;

// Shows how many employees are currently stored on the system.
int added = 0;

// An array of all the employee slots whether they have values behind them or not.
Employee employees[EMPLOYEECOUNT];


// SYNC PHASE
void setup() {

  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.setBacklight(3);
  lcd.createChar(1, upChar);
  lcd.createChar(0, downChar);
  String syncer = "";
  unsigned long firstTime = 0;

// This while loop is used to print R every 2 seconds whilst still being able to detect any serial inputs.
  while (syncer != "BEGIN") {
    if (Serial.available() > 0) {
      syncer = Serial.readStringUntil('\n');
    }
    
    // Checks if begin has been entered so that it can enter the main phase.
    if (syncer == "BEGIN") {
      Serial.println("BASIC");
      lcd.setBacklight(7);
      break;
    } 
    // This section checks if 2 seconds has passed yet so that it can print R.
    else if ((millis() - firstTime) >= 2000) {
      Serial.println('R');
      firstTime = millis();     
    }
  }
}

// MAIN PHASE
void loop() {
  
  static int scrollStart = 0;
  static int scrollEnd = 7;
  static unsigned long timeScroll; 
  static bool begun = false;
  String userPrompt = "";
  bool valid = true;
  static int currentScreen = -1;
  bool finished = false;
  static bool selPressed = false;
  uint8_t buttons = lcd.readButtons();
  unsigned long int startSelect;


// Code regarding the SELECT button being held down   
  
  
  if (buttons == 1){
    if (!selPressed){
      startSelect = millis();
    }
    
    selPressed = true;
    
    // Ensuring that select has been held for over a second before f number is displayed
    if (millis() >= startSelect +1000){
      
      lcd.setBacklight(7);
      lcd.setCursor(0, 0);
      lcd.print("F418623");
      lcd.setCursor(0, 1);
      
      // Used FreeMemory.h library here to find the number of bytes remainuing in RAM
      lcd.print(freeMemory());
      lcd.print(" Bytes remain");
      
    }
    else{
      lcd.clear();
    }
  }else if ((selPressed)&(buttons==0)){
    lcd.clear();
    selPressed = false;
  }
  

  // Functionality for switching between employees using the UP or DOWN buttons.
  
  if (currentScreen !=-1){
    
    // If up button pressed
    if ((buttons == 8) & (currentScreen !=0)){
      currentScreen --;
      timeScroll = millis();
      scrollEnd = 7;
      scrollStart = 0;
      lcd.clear();

    // If down button pressed
    }else if ((buttons == 4) & (currentScreen < (EMPLOYEECOUNT-1)) & (employees[currentScreen+1].grade != -1)){
      timeScroll = millis();
      scrollEnd = 7;
      scrollStart = 0;
      currentScreen ++;
      lcd.clear();
    } 
  }
  

  // Displaying a different screen to normal if the screen is displaying a deleted user.
  if ((employees[currentScreen].jobTitle == "DELETED") & (buttons !=1)& (begun !=false)){
    lcd.setCursor(0, 0);
  
    if (currentScreen == 0){
      lcd.print(" ");
    }
    else{
  	  lcd.write((uint8_t)1);
    }
    lcd.print("ACCOUNT DELETED");
    lcd.setCursor(0, 1);
    if ((currentScreen == EMPLOYEECOUNT)||(employees[currentScreen+1].grade == -1)){
      lcd.print(" ");
    }
    else{
      lcd.write((uint8_t)0);
    }
  }
  
  // Regular employee screen being printed
  else if ((buttons !=1)&(begun != false)){
 
    lcd.setCursor(0, 0); 
    if (currentScreen == 0){
      lcd.print(" ");
    }
    // Printing custom character if there isn't an account above.
    else{
  	  lcd.write((uint8_t)1);
    }


    lcd.print(employees[currentScreen].grade);
    lcd.print(" ");
    
    // Changing backlight colour based upon pension status
    if (employees[currentScreen].pension == "PEN"){
      lcd.print(" ");
      lcd.setBacklight(2);
    }
    else {
      lcd.setBacklight(1);
    }

    lcd.print(employees[currentScreen].pension);
    lcd.print(" ");
  
    lcd.print(employees[currentScreen].salary,2);
    
    lcd.setCursor(0, 1);

    if ((currentScreen == (EMPLOYEECOUNT-1))||(employees[currentScreen+1].grade == -1)){
      lcd.print(" ");
    }
    // Printing a down arrow if there is an account below.
    else{
      lcd.write((uint8_t)0);
    }
    lcd.print(employees[currentScreen].ID);
    lcd.print(" ");
    
    // Printing any shorter job titles.
    if (employees[currentScreen].jobTitle.length() <= 7){
      lcd.print(employees[currentScreen].jobTitle);
    }
    
    // Scrolling mechanism extension for longer job titles.
    else if (millis() - timeScroll >= 1000){
      scrollStart +=1;
      scrollEnd +=1;
      timeScroll = millis();
      if (scrollEnd > employees[currentScreen].jobTitle.length()){
        scrollEnd = 7;
        scrollStart = 0;
      }
    }
    else{
      lcd.print(employees[currentScreen].jobTitle.substring(scrollStart, scrollEnd));
    }
  }

  // Detecting any serial inputs
  if (Serial.available() > 0) {
    userPrompt = Serial.readStringUntil('\n');
    String action = userPrompt.substring(0, 3);
    String userID = userPrompt.substring(4, 11);

    // Checking initial validity of inputs
    if (userPrompt[3] != '-'){
      valid = false;
      Serial.println(F("ERROR: Invalid prompt input"));
    }
    // Ensuring each value in the user ID is numeric 
    bool isNumeric = true;
    for (int i = 0; i < userID.length(); i++) {
      if (!isDigit(userID[i])) {
        isNumeric = false;
        break;
      }
    }
    if (isNumeric != true) {
      Serial.println(F("ERROR: ID entered is invalid"));
      valid = false;
    }
    // Preventing system from spilling over employee cap.
    if (added >= EMPLOYEECOUNT){
      Serial.println(F("ERROR: Maximum number of employees added. Increase EMPLOYEECOUNT to add more."));
      valid = false;
    }

    // Checking if any ID matching the one from the input is already on the system.
    int idExists = -1;
    for (int i = 0; i < EMPLOYEECOUNT; i++) {
      if (employees[i].ID == userID) {
        idExists = i;
        // Ensuring no two employees have the same ID
        if (action == "ADD") {
          Serial.println(F("ERROR: ID already exists"));
          valid = false;
        }
        break;
      }
    }

    // Implementation of adding an employee to the system.
    if ((action == "ADD") & (idExists == -1) & (valid == true)) {
      int grade = userPrompt[12] - '0';

      // Checking that the grade entered is valid.
      if ((grade < 1) || (grade > 9)) {
        valid = false;
        Serial.println(F("ERROR: Grade entered must be between 1 and 9."));
      }
      // Further ensurance that dashes of the input are formatted correctly
      if (userPrompt[13] != '-'){
        Serial.println(F("ERROR: Invalid input format"));
        valid = false;
      }

      // Making sure job title is of valid length
      if ((userPrompt.substring(14, userPrompt.length()).length() >17) || (userPrompt.substring(14, userPrompt.length()).length() <3)){
        Serial.println(F("ERROR: Invalid job title length"));
        valid = false;
      }

      // Ensuring that job title does not have any invalid characters 
      for (int i = 14;i < userPrompt.length();i++){
        if ((userPrompt[i] != '_')&(!isAlphaNumeric(userPrompt[i]))&(userPrompt[i] != '.')){
          valid = false;
          Serial.println(F("Error: Invalid characters are not allowed in job title"));
        }
      }

      // Process of actually adding employee to the system once the input has passed all checks.
      if (valid == true) {
        begun = true;
        // Finds the first empty slot in the array and stores the employee there.
        for (int i = 0; i < EMPLOYEECOUNT; i++) {
          if (employees[i].ID == "") {
            added = added + 1;
            currentScreen = i; // Displays the newly added employee
            employees[i].ID = userID;
            employees[i].grade = grade;
            employees[i].jobTitle = userPrompt.substring(14, userPrompt.length());
            employees[i].pension = "PEN";
            employees[i].salary = 00000.00;
            Serial.println(F("DONE!"));
            lcd.clear();

            break;
          }
        }
      }
    }
    
    else if ((idExists == -1)&(action != "ADD")){
      Serial.println(F("ERROR: Employee ID does not exist."));
    }

    // Changing penstion status and checking that the input is valid.
    else if ((action == "PST") & (idExists != -1)& (valid == true)) {
      if (employees[idExists].salary == 0.0){
        Serial.println(F("ERROR: Enter employee salary before changing pension status."));
      }
      else if (userPrompt.substring(12, (userPrompt.length())) == employees[idExists].pension){
        Serial.println(F("ERROR: Employee already has this pension status."));
      }
      else if ((userPrompt.substring(12, (userPrompt.length())) =="PEN")||(userPrompt.substring(12, (userPrompt.length())) =="NPEN")){
        employees[idExists].pension = userPrompt.substring(12, (userPrompt.length()));
      }
      else{
        Serial.println(F("ERROR: Invalid input"));
      }
    } 

    // Changing pay grade and checking that the input is valid.
    else if ((action == "GRD") & (idExists != -1)& (valid == true)) {
      if (userPrompt.length() < 12){
        Serial.println(F("Error: Invalid input length"));
      } 
      else if (userPrompt[12] <= employees[idExists].grade) {
        Serial.println(F("ERROR: Grade entered cannot be lower than or equal to employee's current grade."));
      } 
      else if ((userPrompt[12] < 1) || (userPrompt[12] > 9)) {
        Serial.println(F("ERROR: Grade entered must be between 1 and 9."));
      }
      else {
        employees[idExists].grade = userPrompt[12];
      }    
    } 
    
    // Changing employee's salary and checking that the input is valid.
    else if ((action == "SAL") & (idExists != -1)& (valid == true)) {
      float sal = userPrompt.substring(12, userPrompt.length()).toFloat();
       
      if ((sal < 99999.99) & (sal>= 0)){      
        employees[idExists].salary = sal;
      }
      else{
        Serial.println(F("ERROR: Invalid salary entered. Must be less than £100000 and not negative"));
      }
    } 

    // Changing employee's job title and checking that the input is valid.
    else if ((action == "CJT") & (idExists != -1)& (valid == true)) {
      bool newValid = true;
      
      // Another for loop used to ensure a job title includes no invalid characters.
      for (int i = 12;i < userPrompt.length();i++){
        if ((!isAlphaNumeric(userPrompt[i]))&(userPrompt[i]!= '_')&(userPrompt[i] != '.')){
          newValid = false;
          Serial.println(F("ERROR: Invalid characters are not allowed in job title"));
        }
      }
      if ((userPrompt.substring(12, userPrompt.length()).length() >17) || (userPrompt.substring(12, userPrompt.length()).length() <3)){
        Serial.println(F("ERROR: Invalid job title length"));
      }
      else if (newValid){
        employees[idExists].jobTitle = userPrompt.substring(12, userPrompt.length());
        lcd.clear();
      }    
    } 

    // Functionality for deleting a user
    else if ((action == "DEL") & (idExists != -1)& (valid == true)) {
      employees[idExists].ID = "";
      employees[idExists].jobTitle = "DELETED";
      employees[idExists].pension = "";
      employees[idExists].grade = 0;
      employees[idExists].salary = 0.00;
      added = added - 1;
      lcd.clear();
    }
  }
}

/*Billy Gallon
3/23/11
Project 3

This program will read in an XML file into a linked list, and then
Check the file for correct syntax.  It will then output the XML
code to a file, formatted for easy reading by a user.*/
#include "stack.h"
#include <iostream>
#include <fstream>
#include <string>
bool KeepChecking()//prompts user to check multiple xml files, to allow for multiple parses before closing the program
{
	char answer;
	cout << "Would you like to parse another file? (Y/N)" << endl;
	cin >> answer;
	if (answer == 'y' || answer == 'Y')
		return true;
	else
		return false;
}
string GetName()//gets filename of file to be checked, so user can check multiple files
{
	string name = "";
	do
	{
	cout << "Please enter the name of the xml file you wish to parse.  Please ensure that thefile is in the same folder as this program, and that you do not add the .xml\nextension at the end of the name.\n";
	cin >> name;
	}
	while (name == "");
	return name;
}
string indentation(int length)//handles indentation of files based on number of tags
{
	string templine;
	for (int i = 0; i < length; i++)
		templine += " ";
	return templine;
}
void lengthCheck(ofstream& outfile, string check)//checks length of output, and splits at 80 characters
{
	string temp;
	if (check.length() > 80)
	{
		temp = check.substr(80, check.length() - 80);
		if (check.find("/>") != string::npos)
		{
			string temp2 = check;
			check = "";
			while(temp.length() <= 80)
			{
				check+=temp2.substr(0,temp2.find_first_of(' '));
				temp2.erase(0,temp2.find_first_of(' '));
			}
		}
		outfile << check.substr(0, 80) << endl;
		if (temp.length() > 80)
			lengthCheck(outfile, temp);
		else
		{
			outfile << temp << endl;
		}
	}
	else
		outfile << check << endl;
}
void error(int lineNum, string header) //gives a generic error message with a line number
{
	cout << "Error on line " << lineNum << " at " << header << endl;
}
bool readFile(string filename, LinkedListType<string>& XMLcode, StackType<string>& headers)
{
	//this function contains all of the code to read a file in, as well as check its syntax and format it for a pretty print output.
	//it first reads in the file to a linked list, and puts headers onto a stack.  As closing headers are found, openign headers
	//are popped from the stack to help ensure correct format.  the correct number of indents are added, and syntax is checked
	//as it is added.  The file will return false if it encounters an error, and stop reading in the file.

	int lineNum = 1;
	bool openCheck = false, versCheck = false, rootCheck = false, stringLitCheck = false;

	//openCheck used for opening of a header, versCheck checks for an XML version at the top of the file,
	//rootCheck makes sure there is a root header surrounding the entire file, and stringLitCheck
	//is used to check if the file is reading in from a string literal, so that it does not
	//incorrectly read syntax

	ifstream infile(filename + ".xml"); //opens a filestream to read the file to be parsed
	infile.unsetf(ios_base::skipws); //forces fstream to read spaces
	char temp;
	string line = "", templine = "", testline, rootHeader = "";
	while (!infile.eof())
	{
		infile >> temp;

		if (temp == '"') //checks for a string literal and flips the flag for entering or exiting
		{
			if (stringLitCheck)
				stringLitCheck = false;
			else if (!stringLitCheck)
				stringLitCheck = true;
		}
		if (temp == '\n') //removes many line breaks
		{
			temp = ' ';
			lineNum++;
		}
		if (temp == '<' && !stringLitCheck) //parses a line if it reads that it is beginning to read an opening header
		{
			if (rootCheck)
			{
				cout << "There is no root header in the XML file " << filename << "." << endl;
				return false;
			}
			if (openCheck)
			{
				error(lineNum, line);
				return false;
			}
			openCheck = true;
			if (line != "")
			{
				XMLcode.AddToRear(templine + line);
				line = "";
			}
		}
		line += temp;
		if (temp == '>' && !infile.eof() && !stringLitCheck)//parses a line if it reads that it is reading the end of a header
		{
			if (!openCheck)
			{
				error(lineNum, line);
				return false;
			}
			openCheck = false;
			testline = line.substr(2, line.size()-2); //gets line to see if closing header matches an opening header
			if (line.find("/>") != string::npos || line.find("<!--") != string::npos && line.find("-->") != string::npos)
			{//if self closing or comment
				XMLcode.AddToRear(templine + line);
			}
			else if (line.find("<?") != string::npos && line.find("?>") != string::npos)
			{//if version
				if (versCheck || XMLcode.LengthIs() != 0)
				{
					error(lineNum, line);
					return false;
				}
				else
					XMLcode.AddToRear(templine + line);
			}
			else if (headers.GetItem().find(testline) != string::npos)
			{//if header is found on stack and matches current header
				if (headers.GetItem().find(rootHeader) != string::npos)
				{
					rootCheck = true;
				}
				headers.Pop();
				templine = indentation(headers.LengthIs());
				XMLcode.AddToRear(templine + line);
			}
			else if (line.find("</") == string::npos)
			{//checks so that it is not pushing closing headers onto the stack
				if (rootHeader == "")
				{
					rootHeader = line;
				}
				templine = indentation(headers.LengthIs());
				headers.Push(line);
				XMLcode.AddToRear(templine + line);
			}
			else 
			{
				if (headers.GetItem().find(line))
				{
					cout << "Error, extra/missing header " << line << " found on line number ";
					cout << lineNum << ".";
				}
				else
				{
					cout << "Error, extra/missing header " << headers.GetItem() << " found after ";
					headers.MoveCurPos();
					cout << headers.GetItem() << ".";
				}
				return false;
			}
			line = "";
		}
	}
	infile.close();
	if (headers.LengthIs() == 0)
		return true;
	else
	{
		cout << "Error, missing header " << headers.GetItem();
		return false;
	}
}
void writeFile(LinkedListType<string>& XMLcode, string filename) //code used to check and correctly output the parsed file to notepad
{
	XMLcode.ResetListForward();
	ofstream outfile(filename + " parsed.xml"); //outputs the new, parsed file as the original name parsed, so it is easy to find
	for (int i = 0; i < XMLcode.LengthIs(); i++)
	{
		lengthCheck(outfile, XMLcode.GetItem());
		XMLcode.MoveCurPos();
	}
	outfile.close();
}
void main()
{
	do
	{
		LinkedListType<string> XMLcode; //Linked list that holds the XML code
		StackType<string> headers; //Stack to hold headers
		string filename = GetName(); //gets the name of the file to parse
		if (!readFile(filename, XMLcode, headers)) //reads the file into an in-memory copy
		{
			//nothing, theres an error in the code
		}
		else
		{//no error
			writeFile(XMLcode, filename); //writes the file out to an xml file
		}
	}
	while (KeepChecking());
}
//THIS PROGRAM IS FOR ABCU ADVISING
//THE PURPOSE OF THE PROGRAM IS TO LOAD DATA FROM A .CSV
//AND STORE DATA IN VARIOUS DATA STRUCTURES FOR USE LATER
//CAN ALSO PRINT OUT DATA IN MULTIPLE SORTING REQUIREMENTS.
//JON SEGUIN
//10-16-2026

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;

// Stores information about a course.
struct Course {
    string courseNumber;
    string title;
    vector<string> prerequisites;
};

// Represents a node in the binary search tree.
struct Node {
    Course course;
    Node* left;
    Node* right;

    Node(const Course& newCourse)
        : course(newCourse), left(nullptr), right(nullptr) {
    }
};

class AdvisingProgram {
private:
    Node* root;
    bool dataLoaded;

    // Converts course numbers to uppercase.
    string toUpperCase(string value) {
        transform(value.begin(), value.end(), value.begin(),
            [](unsigned char c) {
                return static_cast<char>(toupper(c));
            });
        return value;
    }

    // Removes whitespace from the beginning and end of a string.
    string trim(const string& value) {
        size_t first = value.find_first_not_of(" \t\r\n");

        if (first == string::npos) {
            return "";
        }

        size_t last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    // Checks whether a course number contains only letters and digits.
    bool validCourseNumber(const string& number) {
        if (number.empty()) {
            return false;
        }

        for (unsigned char c : number) {
            if (!isalnum(c)) {
                return false;
            }
        }

        return true;
    }

    // Finds a course in the binary search tree.
    Node* searchTree(Node* node, const string& number) {
        if (node == nullptr) {
            return nullptr;
        }

        if (number == node->course.courseNumber) {
            return node;
        }

        if (number < node->course.courseNumber) {
            return searchTree(node->left, number);
        }

        return searchTree(node->right, number);
    }

    // Inserts a course into the binary search tree.
    Node* insertCourse(Node* node, const Course& course) {
        if (node == nullptr) {
            return new Node(course);
        }

        if (course.courseNumber < node->course.courseNumber) {
            node->left = insertCourse(node->left, course);
        }
        else if (course.courseNumber > node->course.courseNumber) {
            node->right = insertCourse(node->right, course);
        }

        return node;
    }

    // Prints courses in alphanumeric order.
    void printInOrder(Node* node) {
        if (node == nullptr) {
            return;
        }

        printInOrder(node->left);

        cout << node->course.courseNumber
            << " - " << node->course.title << '\n';

        printInOrder(node->right);
    }

    // Releases all memory allocated to the tree.
    void deleteTree(Node* node) {
        if (node == nullptr) {
            return;
        }

        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

    // Confirms that every prerequisite exists in the tree.
    bool validatePrerequisites(Node* treeRoot,
        const vector<Course>& courseList) {
        for (const Course& course : courseList) {
            for (const string& prerequisite : course.prerequisites) {
                if (searchTree(treeRoot, prerequisite) == nullptr) {
                    cout << "Error: Course "
                        << course.courseNumber
                        << " has an unknown prerequisite: "
                        << prerequisite << '\n';
                    return false;
                }
            }
        }

        return true;
    }

public:
    AdvisingProgram() : root(nullptr), dataLoaded(false) {
    }

    ~AdvisingProgram() {
        deleteTree(root);
    }

    // Opens CS300.csv automatically and loads the course data.
    void loadCourses() {
        // Hard-coded filename: no user filename prompt.
        ifstream inputFile("CS300.csv");

        if (!inputFile.is_open()) {
            cout << "Error: Unable to open CS300.csv.\n";
            cout << "Place CS300.csv in the program's "
                << "working directory.\n";
            return;
        }

        vector<Course> newCourses;
        string line;
        int lineNumber = 0;

        // Read each line and create a course object.
        while (getline(inputFile, line)) {
            ++lineNumber;

            // Remove the UTF-8 BOM if it appears at the start of the file.
            if (lineNumber == 1 &&
                line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line.erase(0, 3);
            }

            if (trim(line).empty()) {
                continue;
            }

            stringstream stream(line);
            vector<string> fields;
            string field;

            while (getline(stream, field, ',')) {
                fields.push_back(trim(field));
            }

            // Allow a trailing comma to represent an empty final field.
            if (!line.empty() && line.back() == ',') {
                fields.push_back("");
            }

            // Each row must contain a course number and title.
            if (fields.size() < 2 || fields.size() > 4) {
                cout << "Error on line " << lineNumber
                    << ": Expected a course number, title, "
                    << "and up to two prerequisites.\n";
                return;
            }

            Course course;
            course.courseNumber = toUpperCase(fields[0]);
            course.title = fields[1];

            if (!validCourseNumber(course.courseNumber)
                || course.title.empty()) {
                cout << "Error on line " << lineNumber
                    << ": Invalid course number or title.\n";
                return;
            }

            // Read any prerequisite course numbers.
            for (size_t i = 2; i < fields.size(); ++i) {
                string prerequisite = toUpperCase(fields[i]);

                if (!prerequisite.empty()) {
                    if (!validCourseNumber(prerequisite)) {
                        cout << "Error on line " << lineNumber
                            << ": Invalid prerequisite course number.\n";
                        return;
                    }

                    course.prerequisites.push_back(prerequisite);
                }
            }

            // Reject duplicate course numbers.
            for (const Course& existing : newCourses) {
                if (existing.courseNumber == course.courseNumber) {
                    cout << "Error on line " << lineNumber
                        << ": Duplicate course number "
                        << course.courseNumber << ".\n";
                    return;
                }
            }

            newCourses.push_back(course);
        }

        inputFile.close();

        if (newCourses.empty()) {
            cout << "Error: CS300.csv contains no course data.\n";
            return;
        }

        // Build a separate tree so invalid data cannot replace
        // a previously loaded, valid course list.
        Node* newRoot = nullptr;

        for (const Course& course : newCourses) {
            newRoot = insertCourse(newRoot, course);
        }

        // Validate prerequisite references before accepting the file.
        if (!validatePrerequisites(newRoot, newCourses)) {
            deleteTree(newRoot);
            cout << "Course data was not loaded.\n";
            return;
        }

        // Replace the old data after successful validation.
        deleteTree(root);
        root = newRoot;
        dataLoaded = true;

        cout << "Successfully loaded " << newCourses.size()
            << " courses from CS300.csv.\n";
    }

    // Displays the complete course list in alphanumeric order.
    void printCourseList() {
        if (!dataLoaded) {
            cout << "Error: Load the course data first using Option 1.\n";
            return;
        }

        cout << "\nComputer Science Course List\n";
        cout << "----------------------------\n";

        printInOrder(root);
    }

    // Looks up a course and displays its title and prerequisites.
    void printCourseInformation() {
        if (!dataLoaded) {
            cout << "Error: Load the course data first using Option 1.\n";
            return;
        }

        string number;

        cout << "Enter course number: ";
        getline(cin, number);

        number = toUpperCase(trim(number));

        if (!validCourseNumber(number)) {
            cout << "Error: Enter a valid course number.\n";
            return;
        }

        Node* result = searchTree(root, number);

        if (result == nullptr) {
            cout << "Error: Course " << number
                << " was not found.\n";
            return;
        }

        cout << "\nCourse Number: "
            << result->course.courseNumber << '\n';

        cout << "Course Title: "
            << result->course.title << '\n';

        if (result->course.prerequisites.empty()) {
            cout << "Prerequisites: None\n";
        }
        else {
            cout << "Prerequisites:\n";

            for (const string& prerequisite :
                result->course.prerequisites) {
                Node* prereqCourse = searchTree(root, prerequisite);

                cout << "  " << prerequisite;

                if (prereqCourse != nullptr) {
                    cout << " - " << prereqCourse->course.title;
                }

                cout << '\n';
            }
        }
    }

    // Displays the menu and processes the user's selection.
    void run() {
        string choice;

        do {
            cout << "\nABCU Advising Program\n";
            cout << "1. Load Course Data\n";
            cout << "2. Print Course List\n";
            cout << "3. Print Course Information\n";
            cout << "9. Exit\n";
            cout << "Enter your choice: ";

            getline(cin, choice);

            if (choice == "1") {
                loadCourses();
            }
            else if (choice == "2") {
                printCourseList();
            }
            else if (choice == "3") {
                printCourseInformation();
            }
            else if (choice == "9") {
                cout << "Exiting program.\n";
            }
            else {
                cout << "Error: Invalid option. "
                    << "Enter 1, 2, 3, or 9.\n";
            }

        } while (choice != "9");
    }
};

int main() {
    AdvisingProgram program;
    program.run();

    return 0;
}

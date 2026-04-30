#include "SimpleTest/MemoryDiagnostics.h"
#include "SimpleTest/SimpleTest.h"
#include "SimpleTest/TestDriver.h"
#include "SimpleTest/TextUtils.h"
#include <iostream>
#include <cstdlib>
using namespace std;

const vector<string> kTestOrder = {
};

namespace {

    /* Test type --> String */
    string to_string(SimpleTest::TestType type) {
        switch (type) {
            case SimpleTest::TestType::STUDENT: return "Student Test";
            case SimpleTest::TestType::PROVIDED: return "Provided Test";
            case SimpleTest::TestType::AUTOGRADER: return "Autograder Test";
            case SimpleTest::TestType::MANUAL: return "Manual Test";
            default: fatalError("Unknown test type.");
        }
    }

    /* Test --> Display Name */
    string displayNameOf(const SimpleTest::Test& test) {
        return to_string(test.type) + ": " + test.name;
    }

    /* Given a file path, returns the name of the file, excluding the path to it. */
    string tailOf(const string& path) {
        /* Find the last / or \ in the filename. */
        size_t index = path.find_last_of("\\/");
        return index == string::npos ? path : path.substr(index + 1);
    }
    
    SimpleTest::TestGroupComparator comparator() {
        return [](const string& lhs, const string& rhs) {
            return find(kTestOrder.begin(), kTestOrder.end(), tailOf(lhs)) <
                   find(kTestOrder.begin(), kTestOrder.end(), tailOf(rhs));
        };
    }

    /* Retrieves all files that contain tests, i.e. all test groups */
    vector<string> getTestGroups() {
        vector<string> keys;
        auto& tests = SimpleTest::Internal::rawTests();

        for (const auto& entry: tests) {
            keys.push_back(tailOf(entry.first));
        }
        
        sort(keys.begin(), keys.end(), comparator());
        return keys;
    }
    
    string getLine(const string& prompt) {
        cout << prompt;
        string result;
        getline(cin, result);
        return result;
    }
    
    int getInteger(const string& prompt) {
        while (true) {
            string line = getLine(prompt);
            
            istringstream converter(line);

            int result;            
            converter >> result;
            if (converter) {
                char leftover;
                converter >> leftover;
                
                if (!converter) return result;
            }
            
            cerr << "Please enter an integer." << endl;
        }
    }

    /* Prompts the user via the console for which test group to run (or all of them). -1 -> all. */
    int getTestSelection(const vector<string>& groups) {
        /* User isn't asked if there's just one set of tests to run. */
        if (groups.size() == 1) return 0;
    
        cout << "Select which test to run: " << endl;

        cout << "0 All Tests" << endl;
        for (size_t i = 0; i < groups.size(); i++) {
            cout << i + 1 << " " << groups[i] << endl;
        }

        return getInteger("Select which test to run: ") - 1;
    }

    /* Returns an appropriate TestFilter given the Vector of test groups. */
    SimpleTest::TestFilter getTestFilter(const vector<string>& groups, int selection) {
        if (selection < 0) {
            return [](const string&, const SimpleTest::Test&) { return true; };
        }

        string selectedGroup = groups[selection];

        return [selectedGroup](const string& group, const SimpleTest::Test&) {
            return group == selectedGroup;
        };
    }
}

int main() {
    auto groups = getTestGroups();
    int selection = getTestSelection(groups);

    vector<SimpleTest::TestGroup> lastGroups;
    const SimpleTest::Test* running = nullptr;
    SimpleTest::TestReporter reporter = [&](const vector<SimpleTest::TestGroup>& groups) {
        /* Stash the last set of groups so that we can print failures later. */
        lastGroups = groups;
        for (const auto& group: groups) {
            for (const auto& test: group.tests) {
                if (running == &test) {
                    if (test.result == SimpleTest::TestResult::PASS) {
                        cout << "    pass" << endl;
                    } else if (test.result == SimpleTest::TestResult::FAIL) {
                        cout << "    FAIL: " << test.detailMessage << endl;
                    } else if (test.result == SimpleTest::TestResult::EXCEPTION) {
                        cout << "    FAIL: " << test.detailMessage << endl;
                    } else if (test.result == SimpleTest::TestResult::LEAK) {
                        cout << "    LEAK: " << test.detailMessage << endl;
                    } else {
                        fatalError("Internal error: Unknown test result?");
                    }
                    running = nullptr;
                }
                if (test.result == SimpleTest::TestResult::RUNNING) {
                    running = &test;
                    cout << "Running " << displayNameOf(test) << " (" << group.name << ")" << endl;
                }
            }
        }
    };

    SimpleTest::run(reporter, getTestFilter(groups, selection), comparator());

    cout << endl;
    cout << "Test summary: " << endl;

    /* List error counts. */
    int totalTests = 0;
    int totalPassed = 0;
    for (const auto& group: lastGroups) {
        if (selection < 0 || groups[selection] == group.name) {
            cout << group.name << ": " << group.numPassed << " of " << pluralize(group.numTests, "test") << " passed." << endl;
            
            totalTests += group.numTests;
            totalPassed += group.numPassed;
        }
    }

    if (selection < 0) {
        cout << "Overall: " << totalPassed << " of " << pluralize(totalTests, "test") << " passed." << endl;
    }

    if (totalTests == totalPassed) {
        cout << "All tests passed!" << endl;
    }
}

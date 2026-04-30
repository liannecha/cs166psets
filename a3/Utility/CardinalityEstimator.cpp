#include "CardinalityEstimator.h"
#include <iostream>
#include <map>
using namespace std;

/***** Registry *****/
namespace Registry {
    /* The actual, internal registry. We use a static local variable to avoid static
     * initialization order bugs.
     */
    namespace {
        map<string, Constructor>& estimators() {
            static map<string, Constructor> theRegistry;
            return theRegistry;
        }
    }
    
    void addEstimator(const string& name, Constructor constructor) {
        estimators()[name] = constructor;
    }
    
    Constructor estimatorNamed(const string& name) {
        auto result = estimators().find(name);
        if (result != estimators().end()) return result->second;
        
        /* Oops, not here. Show the possibilities and abort. */
        cerr << "There is no estimator type named '" << name << "'. Options are: " << endl;
        for (const auto& [key, _]: estimators()) {
            cerr << "  " << key << endl;
        }
        throw invalid_argument("No estimator with the given name exists.");
    }
}

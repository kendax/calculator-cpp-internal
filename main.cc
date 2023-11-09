#include <drogon/drogon.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <type_traits>
#include <algorithm>

using namespace drogon;
using namespace std;

/* Define a map that holds the calculator operators each assigned an integer indicating its precedence in 
a mathematical expression */
std::unordered_map<std::string, int> precedence = {
    {"+", 1},
    {"-", 1},
    {"*", 2},
    {"/", 2},
    {"*-", 3}
};

//Check if a string is an operator or a negative sign
bool isOperator(const std::string& token) {
    auto it = precedence.find(token);

    // Check if the token exists in the precedence map or if it's a single "-"
    bool ok = (it != precedence.end()) || (token == "-" && token.length() == 1);
    
    return ok;
}

//Check if a string itself is an operator 
bool isSpecialCharacter(string c) {
    // Define a set of operators
    std::string operators = "([*\\-+*/()])";

    // Check if the character is in the set of operators
    return operators.find(c) != std::string::npos;
}

// Function to parse a number from a string to a double
double parseNumber(const std::string& token) {
    double value;
    std::istringstream(token) >> value;
    return value;
}

// Add whitespace anywhere there is an operator thereby separating numbers and operators
std::string addWhitespaceAroundOperators(const std::string& input) {
    // Define a regular expression pattern to match operators
    std::string pattern = "([*\\-+*/()])";

    try {
        //compile the regular expression
        std::regex regex(pattern);

        //Use the regular expression to find matches and add whitespace around them
        std::string result = std::regex_replace(input, regex, " $1 ");
        return result;
    } catch (std::regex_error& e) {
        std::cerr << "Error compiling regular expression: " << e.what() << std::endl;
        return input;
    }
}

//Check if a variable is an integer or a floating point variable
template <typename T>
bool canBeCounted(const T& value) {
    return std::is_integral<T>::value || std::is_floating_point<T>::value;
}

// Read the values from the 'input' vector and write them to a single string variable
std::string getInputAsString(const std::vector<std::string>& values) {
    std::string result;

    for (const std::string& value : values) {
        result += value;
    }

    return result;
}

//Check if a string contains any operator
bool containsSpecialCharacters(const std::string &str) {
    // Define a regular expression pattern to match operators
    std::regex operators("([*\\-+*/()])"); // This matches any operators

    // Use std::regex_search to check if the string contains any operators
    return std::regex_search(str, operators);
}

//Check if the elements of a vector contain an operator
bool foundOperatorFunc(const std::vector<std::string> &vec) {
    for (const std::string &str : vec) {
        if (containsSpecialCharacters(str)) {
            return true; // Found a string with an operator
        }
    }
    return false;  // No string with an operator found in the vector
}

//Check if any of the elements of a vector is a period
bool periodInInputFunc(const std::vector<std::string> &vec) {
    for (const std::string &str : vec) {
        if (str.find('.') != std::string::npos) {
            return true; // Found a string with a period
        }
    }
    return false; // No string with a period found in the vector
}

//Check whether there is a period in a the last number occuring after an operator
bool periodInLastOperand(const std::vector<std::string>& input) {
    int lastOperatorIndex = -1;

    // Find the index of the last operator in the vector
    for (int i = 0; i < input.size(); ++i) {
        if (isSpecialCharacter(input[i])) {
            lastOperatorIndex = i;
        }
    }

    // If an operator is found, check for a period in the substring from that index to the end
    if (lastOperatorIndex != -1) {
        for (int i = lastOperatorIndex; i < input.size(); ++i) {
            if (input[i] == ".") {
                return true;
            }
        }
    }

    return false;
}

//Remove all zeros occuring after a decimal point as they have no relevance
std::string removeTrailingZeros(double number) {
    // Convert the double to a string with fixed precision using an output string stream
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(15) << number;
    std::string numStr = oss.str();

    //Remove trailing zeros
    size_t pos = numStr.find_last_not_of('0');
    if (pos != std::string::npos) {
        numStr = numStr.substr(0, pos + 1);
    }

    // Check if the number ends with a decimal point and remove it
    if (!numStr.empty() && numStr.back() == '.') {
        numStr.pop_back();
    }

    return numStr;
}

//Evaluate an expression based on the provided operators
std::pair<std::vector<std::string>, std::vector<double>> performOperation(
    std::vector<std::string> &operatorStack,
    std::vector<double> &valueStack
) {
    std::string op = operatorStack.back();
    operatorStack.pop_back();

    double operand2 = valueStack.back();
    valueStack.pop_back();

    double operand1 = valueStack.back();
    valueStack.pop_back();

    double result = 0.0;

    switch (op[0]) {
        case '*':
            if (op.size() > 1 && op[1] == '-') {
                result = operand1 * (operand2 * -1);
            } else {
                result = operand1 * operand2;
            }
            break;
        case '+':
            result = operand1 + operand2;
            break;
        case '-':
            result = operand1 - operand2;
            break;
        case '/':
            result = operand1 / operand2;
            break;
        default:
            break;
    }

    valueStack.push_back(result);

    return std::make_pair(operatorStack, valueStack);
}

//A function to receive and prepare a string for evaluation as a mathematical expression
std::pair<double, std::string> calculateInput(const std::string& expression) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(expression); //Pass the expression into an input string stream

    //Pass the expression into an input string stream
    while (iss >> token) {
        tokens.push_back(token);
    }

    std::vector<std::string> operatorStack;
    std::vector<double> valueStack;
    
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];

        if (token == "(") {
            operatorStack.push_back(token);
        } else if (token == ")") {
            while (!operatorStack.empty() && operatorStack.back() != "(") {
                std::tie(operatorStack, valueStack) = performOperation(operatorStack, valueStack);
            }
            operatorStack.pop_back(); // Pop "("
        } else if (isOperator(token)) {
            if (token == "-" && (i == 0 || isOperator(tokens[i - 1]) || tokens[i - 1] == "(")) {
                valueStack.push_back(-1.0);
                operatorStack.push_back("*");
            } else {
                while (!operatorStack.empty() && precedence[operatorStack.back()] >= precedence[token]) {
                    std::tie(operatorStack, valueStack) = performOperation(operatorStack, valueStack);
                }
                operatorStack.push_back(token);
            }
        } else {
            try {
                double value = parseNumber(token);
                valueStack.push_back(value);
            } catch (...) { // catch any error in converting the token string to a double
                return std::make_pair(0.0, "Invalid expression");
            }
        }
    }

    while (!operatorStack.empty()) {
        std::tie(operatorStack, valueStack) = performOperation(operatorStack, valueStack);
    }
    //If there is a value in the valueStack vector, return it
    if (valueStack.size() == 1) {
        return std::make_pair(valueStack[0], "");
    }

    //if the calculation was not successful return "Invalid expression"
    return std::make_pair(0.0, "Invalid expression");
}

//Declare an "input" vector that will hold all the values in the input box
std::vector<std::string> input;

int main()
{
    //Handle displaying of dynamic data in the HTML template
    app().registerHandler(
        "/",
        [](const HttpRequestPtr &req,
            std::function<void(const HttpResponsePtr &)> &&callback) {
                /*Create a dataset called 'data' using the class "HttpViewData" and insert into it the values that will
                be displayed in the html input box in the html template */
                HttpViewData data;
                data.insert("input", getInputAsString(input));

                // Go to the html template along with the dataset
                auto resp = HttpResponse::newHttpViewResponse("/Template.csp", data);
                callback(resp);
            },
            {Get}); //Using GET method for this function
    
    app().registerHandler(
        "/postinput",
         [](const HttpRequestPtr &req,
            std::function<void(const HttpResponsePtr &)> &&callback) {
                const auto &formParams = req->getParameters(); //Get all the parameters of the buttons in the form
                std::string equals = req->getParameter("equals");
                req->session()->insert("isOperator", false);
                std::string pattern = "([*\\-|+|/|\\(|\\)])"; // Define the regular expression pattern
                std::regex operatorPattern(pattern); //compile the regular expression
                bool foundOperator;
                int operatorPosition = -1;
                bool lastElementOperator;
                //Check if there is an operator in the input vector and get its position
                for (int index = 0; index < input.size(); ++index) {
                    const std::string& item = input[index];
                    //Check if there is an operator in the input box and if present get its position
                    if (std::regex_search(item, operatorPattern)) {
                        operatorPosition = index;
                    }
                }

                //Loop over every button clicked on the html page
                for (const auto &param : formParams) {
                    const std::string &key = param.first; //assign the first bit of every button's parameters to a constant reference and call it key
                    const std::string &value = param.second; //assign the second bit of every button's parameters to a constant reference and call it value
                    //Check if the button pressed is an operator
                    bool valueIsOperator = std::regex_search(value, operatorPattern);
                    //Check if the button pressed is an operator and the input box was already showing a result
                    if ((key == "divide" || key == "multiply" || key == "minus" || key == "add") && req->getSession()->get<bool>("resultDisplayed") == true) {
                        req->session()->erase("isOperator");
                        req->session()->insert("isOperator", true);
                        req->session()->erase("resultDisplayed");
                        req->session()->insert("resultDisplayed", false); // negate this session variable
                    }

                    //If the percentage key has been clicked
                    if (key == "modulus") {
                        int indexToSlice = operatorPosition + 1;

                        //if the input vector is empty redirect to the html page and show an empty input box
                        if (input.size() == 0) {
                            input.clear();
                            auto resp = HttpResponse::newRedirectionResponse("/");
                            callback(resp);
                        }
                        // slice the input vector from just after the operator till the end to obtain the last operand
                        std::vector<std::string> slicedInput(input.begin() + indexToSlice, input.end());

                        //convert the vector to a string
                        std::string implodeToPercentage;
                        for (const std::string& str : slicedInput) {
                            implodeToPercentage += str;
                        }

                        double toPercentage;
                        // attempt to convert the string to a double and catch any errors
                        try {
                            toPercentage = std::stod(implodeToPercentage);
                        } catch (const std::invalid_argument& e) {
                            std::cerr << "Invalid argument: " << e.what() << std::endl;
                            input.clear();
                            input.push_back("Error");
                            auto resp = HttpResponse::newRedirectionResponse("/");
                            callback(resp);
                            return;
                        } catch (const std::out_of_range& e) {
                            std::cerr << "Out of range: " << e.what() << std::endl;
                            input.clear();
                            input.push_back("Error");
                            auto resp = HttpResponse::newRedirectionResponse("/");
                            callback(resp);
                            return;
                        }

                        toPercentage /= 100; // divide the last operand by 100  to get its value as a percentage
                        std::vector<std::string> operandWithoutModulus;
                        // slice the input vector upto and including the operator
                        if (indexToSlice > 0) {
                            operandWithoutModulus.assign(input.begin(), input.begin() + indexToSlice);
                        }

                        std::string firstPattern = "([*\\-+*/()])"; //pattern for a plus or minus
                        std::regex firstOperatorPattern(firstPattern);
                        std::string secondPattern = "([*/()])"; // pattern for division or multiplication
                        std::regex secondOperatorPattern(secondPattern);
                        bool plusInInput = false, divisionInInput = false, negativeInInput = false;
                        //loop over the input vector as "elements"
                        for (const std::string& element : input) {
                            if (req->getSession()->get<bool>("multiAndMinus") == true) {
                                negativeInInput = true;
                            } else if (std::regex_search(element, secondOperatorPattern)) {
                                divisionInInput = true;
                            } else if (std::regex_search(element, firstOperatorPattern)) {
                                plusInInput = true;
                            }
                        }

                        if (plusInInput) { // if the modulus expression contains a plus or minus
                            std::vector<std::string> operandWithoutOperator(input.begin(), input.begin() + indexToSlice);
                            std::string implodedOperandStr;
                            for (const std::string& str : operandWithoutOperator) {
                                implodedOperandStr += str;
                            }

                            double implodedOperand;
                            try {
                                implodedOperand = std::stod(implodedOperandStr);
                            } catch (const std::invalid_argument& e) {
                                std::cerr << "Invalid argument: " << e.what() << std::endl;
                                input.clear();
                                input.push_back("Error");
                                auto resp = HttpResponse::newRedirectionResponse("/");
                                callback(resp);
                            } catch (const std::out_of_range& e) {
                                std::cerr << "Out of range: " << e.what() << std::endl;
                                input.clear();
                                input.push_back("Error");
                                auto resp = HttpResponse::newRedirectionResponse("/");
                                callback(resp);
                            }

                            double operandWithModulus = implodedOperand * toPercentage;
                            operandWithoutModulus.push_back(std::to_string(operandWithModulus));
                            std::string operandWithoutModulusStr;
                            for (const std::string &str : operandWithoutModulus) {
                                operandWithoutModulusStr += str;
                            }

                            std::string currentValueSpc = addWhitespaceAroundOperators(operandWithoutModulusStr);
                            std::pair<double, std::string> currentValuePair = calculateInput(currentValueSpc);
                            double currentValue;
                            if (currentValuePair.second.empty()) {
                                currentValue = currentValuePair.first;
                                std::cout << "Result: " << currentValuePair.first << std::endl;
                            } else {
                                std::cerr << currentValuePair.second << std::endl;
                            }

                            input.clear();
                            input.push_back(removeTrailingZeros(currentValue));
                            req->session()->erase("resultDisplayed");
                            req->session()->insert("resultDisplayed", true);
                        } else if (divisionInInput) { //if the modulus expression contains a multiplication or division symbol
                            operandWithoutModulus.push_back(std::to_string(toPercentage));
                            std::string operandWithoutModulusStr;
                            for (const std::string & str : operandWithoutModulus) {
                                operandWithoutModulusStr += str;
                            }

                            std::string currentValueSpc = addWhitespaceAroundOperators(operandWithoutModulusStr);
                            std::pair<double, std::string> currentValuePair = calculateInput(currentValueSpc);
                            double currentValue;
                            if (currentValuePair.second.empty()) {
                                currentValue = currentValuePair.first;
                                std::cout << "Result: " << currentValuePair.first << std::endl;
                            } else {
                                std::cerr << currentValuePair.second << std::endl;
                            }

                            input.clear();
                            input.push_back(removeTrailingZeros(currentValue));
                            req->session()->erase("resultDisplayed");
                            req->session()->insert("resultDisplayed", true);
                        } else if (negativeInInput) { // if the modulus expression contains a multiplication sign followed by a minus sign
                            operandWithoutModulus.push_back(std::to_string(toPercentage));
                            std::string operandWithoutModulusStr;
                            for (const std::string& str : operandWithoutModulus) {
                                operandWithoutModulusStr += str;
                            }

                            std::string currentValueSpc = addWhitespaceAroundOperators(operandWithoutModulusStr);
                            std::pair<double, std::string> currentValuePair = calculateInput(currentValueSpc);
                            double currentValue;
                            if (currentValuePair.second.empty()) {
                                currentValue = currentValuePair.first;
                                std::cout << "Result: " << currentValuePair.first << std::endl;
                            } else {
                                std::cerr << currentValuePair.second << std::endl;
                            }

                            input.clear();
                            input.push_back(removeTrailingZeros(currentValue));
                            req->session()->erase("resultDisplayed");
                            req->session()->insert("resultDisplayed", true);
                            req->session()->erase("multiAndMinus");
                            req->session()->insert("multiAndMinus", false);
                        } else { // if the modulus expression has no expression and is just a number
                            double currentValueFlt = std::stod(getInputAsString(input));
                            currentValueFlt /= 100;
                            input.clear();
                            input.push_back(removeTrailingZeros(currentValueFlt));
                            req->session()->erase("resultDisplayed");
                            req->session()->insert("resultDisplayed", true);
                        }
                    } else if (key == "equals") { //if the equals button has been clicked
                            std::string inputConv;
                            for (const std::string& str : input) { // convert the vector to a string
                                inputConv += str;
                            }

                            //if the input box is empty redirect back to the html page
                            if (inputConv == "") {
                                auto resp = HttpResponse::newRedirectionResponse("/");
                                callback(resp);
                                return;
                            }

                            std::string currentValueSpc = addWhitespaceAroundOperators(inputConv);
                            std::pair<double, std::string> currentValuePair = calculateInput(currentValueSpc);
                            double currentValue;
                            if (currentValuePair.second.empty()) {
                                currentValue = currentValuePair.first;
                                std::cout << "Result: " << currentValuePair.first << std::endl;
                            } else {
                                std::cerr << currentValuePair.second << std::endl;
                                input.clear();
                                input.push_back(currentValuePair.second);
                                req->session()->erase("resultDisplayed");
                                req->session()->insert("resultDisplayed", true);
                                req->session()->erase("isOperator");
                                req->session()->insert("isOperator", false);
                                auto resp = HttpResponse::newRedirectionResponse("/");
                                callback(resp);
                                return;
                            }

                            //if the answer starts with a period, append a zero before it
                            if (!std::to_string(currentValue).empty() && std::to_string(currentValue)[0] == '.') {
                                std::string currentValueStr = "0" + std::to_string(currentValue);
                                double currentValueFlt = std::stod(currentValueStr);
                                currentValue = currentValueFlt;
                            }

                            input.clear();
                            input.push_back(removeTrailingZeros(currentValue));
                            req->session()->erase("resultDisplayed");
                            req->session()->insert("resultDisplayed", true);
                            req->session()->erase("isOperator");
                            req->session()->insert("isOperator", false);
                    } else if (key == "c") { // if the C button is clicked
                            input.clear();
                            if (lastElementOperator == true) {
                                lastElementOperator = false;
                            }

                            req->session()->erase("resultDisplayed");
                            req->session()->insert("resultDisplayed", false);
                            req->session()->erase("isOperator");
                            req->session()->insert("isOperator", false);
                            req->session()->erase("multiAndMinus");
                    } else if (key == "delete") { // if the delete button has been clicked
                            if (input.size() > 0) {
                                if (lastElementOperator) {
                                    lastElementOperator = false;
                                }
                                if (lastElementOperator && req->getSession()->get<bool>("multiAndMinus") == true) {
                                    req->session()->erase("multiAndMinus");
                                }
                                input.pop_back();
                            }
                            req->session()->erase("resultDisplayed");
                            req->session()->insert("resultDisplayed", false);
                            req->session()->erase("isOperator");
                            req->session()->insert("isOperator", false);
                    } else { // for any other button clicked
                            std::string lastElement;
                            int secondLastElement = -1;
                            if (input.size() > 0) {
                                lastElement = input.back(); //get the last element in the input vector
                                if (isSpecialCharacter(lastElement)) { // check if the last element in the input vector is an operator
                                    lastElementOperator = true;
                                }
                            }
                            
                            if (input.size() > 2) { //get the index of the second last element
                                secondLastElement = input.size() - 2;
                            }

                            bool secondLastMultiply = false;
                            //check if the second last element is a multiplication symbol
                            if (secondLastElement != -1 && input[secondLastElement] == "*") {
                                secondLastMultiply = true;
                            }

                            //If the period button has been clicked and there is no number preceeding it, append a zero before the period
                            if (key == "period" && (input.size() < 1 || lastElementOperator)) {
                                input.push_back("0");
                            }

                            if (lastElement == "Invalid expression" || lastElement == "Error") { // if "Invalid expression" or "Error" is being shown and another button is clicked, first clear the input box
                                input.clear();
                            }

                            //In the case that the program was expecting a negative number as the first operand but other operators are provided instead, clear the input box
                            if (lastElement == "-" && input.size() == 1 && (key == "divide" || key == "multiply" || key == "add")) {
                                input.clear();
                            }

                            if ((key == "divide" || key == "multiply" || key == "add") && input.size() < 1 ) {/*Prohibit an operator
                                being provided as the first value in the input box*/
                                input.clear();
                            } else if (lastElement == "*" && value == "-") { //Allow the minus operator to follow the multiplication operator
                                req->session()->insert("multiAndMinus", true);
                                input.push_back(value);
                            } else if (secondLastMultiply && lastElement == "-" && (key == "divide" || key == "multiply" || key == "add")) { /*If any operator comes immediately after
                                a multiplication sign and a minus sign, remove the latter two and insert that operator into the input box */
                                if (input.size() >= 2) {
                                    //input.assign(input.begin(), input.begin()+ input.size()-2);
                                    input.pop_back();
                                    input.pop_back();
                                    input.push_back(value);
                                }
                            } else if ((key == "divide" || key == "multiply" || key == "add" || key == "minus") && lastElementOperator) { //Prevent two operators from following each other
                                    if (input.size() > 0) {
                                        input.assign(input.begin(), input.begin() + input.size() - 1);
                                    }
                                    input.push_back(value);
                            } else if (req->getSession()->get<bool>("resultDisplayed") == true) { /*If any number is clicked while the input box is displaying an
                                    answer, erase and start inserting values afresh */
                                    input.clear();
                                    input.push_back(value);
                                    req->session()->erase("resultDisplayed");
                                    req->session()->insert("resultDisplayed", false);
                            } else if (((periodInInputFunc(input) && !foundOperatorFunc(input)) || periodInLastOperand(input)) && key == "period") { //Prevent any operand from having more than one period
                                    input = input;
                            } else { //Add the value of the clicked button to the input array
                                    input.push_back(value);
                            }
                    }
                }
                // redirect to the root url
                auto resp = HttpResponse::newRedirectionResponse("/");
                callback(resp);
            },
            {Post}); //Using POST method for this function

    LOG_INFO << "Server running on 0.0.0.0:8080"; //Display in the terminal the program's address and port while running
    
    //Set HTTP listener address and port and run the HTTP framework
    app()
        // All sessions are stored for 24 Hours
        .enableSession(24h)
        .addListener("0.0.0.0", 8080)
        .run();
}

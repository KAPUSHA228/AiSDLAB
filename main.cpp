#include <string>
#include "Scripts/Lexer.h"
#include <fstream>
#include "Scripts/json.hpp"
#include "Flowchart/FromExpressions.h"
#include "Flowchart/ExporterJson.h"
#include "CodeGen/PascalCodeGenerator.h"
#include "CodeGen/CppCodeGenerator.h"
#include "Parser/PascalParserToExpression.h"
#include "Parser/CParserToAST.h"
#include "Parser/AstVisualizer.h"
#include "Parser/ASTParserToJSON.h"

using namespace std;

int main() {
    //    string CPlusPlusCode = "return 0;";

    string pascalCode =
            "program qq;"
            "function AddNumbers ( a , b : integer ) : integer ;"
            "begin"
            "AddNumbers := a + b;"
            "end;"
            "procedure GreetUser ( name : string ) ;"
            "begin"
            "Writeln ( 'Hello,' , name , '!' );"
            "end;"
            "const"
            "PI: real = 3.1415926;"
            "var"
            "num1, num2, i: integer;"
            "Res, d: real;"
            "res2: string;"
            "begin"
            "Read ( Res ) ;"
            "Writeln ('From Read ' , Res ) ;"
            "num1 := 12 div 2;"
            "num1 := AddNumbers ( PI , PI);"
            "if 5 mod 3 > 0 then begin"
            "Writeln ('Yes,if 1 ');"
            "Writeln ('Yes,if 2 ');"
            "end"
            "case num1 of"
            "begin"
            "1 , 2 , 3 :"
            "begin"
            "Writeln ( 'Switch works 1' );"
            "end;"
            "4 , 5 :"
            "begin"
            "Writeln ( 'Switch works 2' );"
            "end;"
            "else :"
            "begin"
            "Writeln ( 'Switch no works' );"
            "end;"
            "end;"
            "if 5 mod 3 > 0 then begin"
            "Writeln ('Yes,if 1 ');"
            "Writeln ('Yes,if 2 ');"
            "end"
            "else begin"
            "Writeln ('No,else 1 ');"
            "Writeln ('No,else 2 ');"
            "end;"
            "if 5 mod 3 > 0 then begin"
            "Writeln ('Yes,if 1 ');"
            "Writeln ('Yes,if 2 ');"
            "end"
            "else begin"
            "Writeln ('No,else 1 ');"
            "Writeln ('No,else 2 ');"
            "end;"
            "res2 := 'Hello world' ;"
            "num1 := 2 ;"
            "Writeln ('From table ' , num1);"
            "if PI <> num1 then begin"
            "Writeln( 'Pim' );"
            "if PI <> num1 then begin"
            "Writeln ('Pam');"
            "end"
            "end"
            "else begin"
            "Writeln ('Pum');"
            "end;"
            "if PI <> num1 then begin"
            "Writeln( 'Pim' );"
            "if 5 mod 3 > 0 then begin"
            "Writeln ('Yes,if 1 ');"
            "Writeln ('Yes,if 2 ');"
            "end"
            "else begin"
            "Writeln ('No,else 1 ');"
            "Writeln ('No,else 2 ');"
            "end;"
            "end"
            "else begin"
            "Writeln ('Pum');"
            "end;"
            "for i := 1 to 8 do begin"
            "Write ( '3' ) ;"
            "end;"
            "while num1 < 6 do begin"
            "Write('Yes');"
            "num1 := num1 + 1;"
            "end;"
            "if 5 mod 3 > 0 then begin"
            "Writeln ('Yes,if 1 ');"
            "Writeln ('Yes,if 2 ');"
            "end"
            "else begin"
            "Writeln ('No,else 1 ');"
            "Writeln ('No,else 2 ');"
            "end;"
            "repeat begin"
            "Write('3');"
            "num1 := num1 + 1;"
            "end;"
            "until num1 < 7 ;"
            "num1 := num1 + 3;"
            "end.";

    string cCode =
            "#include <stdio.h>\n"
            "#include <stdlib.h>\n"
            "#include <string.h>\n"
            "#define PI 3.1415926\n"
            "#define MAX_SIZE 100\n"
            "int addNumbers(int a, int b);\n"
            "void greetUser(char* name);\n"
            "float calculateCircleArea(float radius);\n"
            "\n"
            "const float PI_CONST = 3.1415926f;\n"
            "int globalCounter = 0;\n"
            "\n"
            "struct Point {\n"
            "    int x;\n"
            "    int y;\n"
            "};\n"
            "\n"
            "typedef struct Point Point;\n"
            "\n"
            "int main() {\n"
            "    int num1, num2, i;\n"
            "    float res, d;\n"
            "    char res2[50];\n"
            "    int array[10];\n"
            "    Point p1;\n"
            "    printf(\"Enter a number: \");\n"
            "    scanf(\"%f\", &res);\n"
            "    printf(\"From input: %.2f\\n\", res);\n"
            "    num1 = 12 / 2;\n"
            "    num2 = 15 % 4;\n"
            "    num1 = addNumbers(5, 3);\n"
            "    switch(num1) {\n"
            "        case 1:\n"
            "        case 2:\n"
            "        case 3:\n"
            "        case 4:\n"
            "        case 5:\n"
            "            printf(\"Switch works\\n\");\n"
            "            break;"
            "        default:\n"
            "            printf(\"Switch no works\\n\");\n"
            "            break;\n"
            "    }\n"
            "    if (5 % 3 > 0) {\n"
            "        printf(\"Yes,if 1\\n\");\n"
            "        printf(\"Yes,if 2\\n\");\n"
            "    } else {\n"
            "        printf(\"No,else 1\\n\");\n"
            "        printf(\"No,else 2\\n\");\n"
            "    }\n"
            "    if (5 % 3 > 0) {\n"
            "        printf(\"Yes,if 1\\n\");\n"
            "        printf(\"Yes,if 2\\n\");\n"
            "    } else {\n"
            "        printf(\"No,else 1\\n\");\n"
            "        printf(\"No,else 2\\n\");\n"
            "    }\n"
            "    strcpy(res2, \"Hello world\");\n"
            "    num1 = 2;\n"
            "    if (5 % 3 == 0) {\n"
            "        printf(\"Yes,if 2\\n\");\n"
            "    } else {\n"
            "        printf(\"No,else 2\\n\");\n"
            "    }\n"
            "    printf(\"From table: %d\\n\", num1);\n"
            "    if (PI_CONST != num1) {\n"
            "        printf(\"Pim\\n\");\n"
            "        if (PI_CONST != num1) {\n"
            "            printf(\"Pam\\n\");\n"
            "        }\n"
            "    } else {\n"
            "        printf(\"Pum\\n\");\n"
            "    }\n"
            "    for (i = 1; i <= 8; i++) {\n"
            "        printf(\"3\");\n"
            "    }\n"
            "    printf(\"\\n\");\n"
            "    while (num1 < 6) {\n"
            "        printf(\"Yes\");\n"
            "        num1 = num1 + 1;\n"
            "    }\n"
            "    printf(\"\\n\");\n"
            "    do {\n"
            "        printf(\"3\");\n"
            "        num1 = num1 + 1;\n"
            "    } while (num1 < 7);\n"
            "    printf(\"\\n\");\n"
            "    for (i = 0; i < 10; i++) {\n"
            "        array[i] = i * 2;\n"
            "    }\n"
            "    p1.x = 10;\n"
            "    p1.y = 20;\n"
            "    int* ptr = &num1;\n"
            "    *ptr = *ptr + 3;\n"
            "    num1 = num1 + 3;\n"
            "    num1 += 5;\n"
            "    num1++;\n"
            "    if (num1 > 10 && num2 < 20 || !(num1 == 15)) {\n"
            "        printf(\"Complex condition works\\n\");\n"
            "    }\n"
            "    num1 = num1 & 0xFF;\n"
            "    num2 = num1 | 0x0F;\n"
            "    int result = (num1 > num2) ? num1 : num2;\n"
            "    greetUser(\"John\");\n"
            "    float area = calculateCircleArea(5.0f);\n"
            "    printf(\"Circle area: %.2f\\n\", area);\n"
            "    return 0;\n"
            "}\n"
            "int addNumbers(int a, int b) {\n"
            "    return a + b;\n"
            "}\n"
            "void greetUser(char* name) {\n"
            "    printf(\"Hello, %s!\\n\", name);\n"
            "}\n"
            "float calculateCircleArea(float radius) {\n"
            "    return PI_CONST * radius * radius;\n"
            "}";
    char *tests[21];
    tests[0] =
            "#include <stdio.h>\n"
            "#include <stdlib.h>\n"
            "#include <string.h>\n"
            "#define PI 3.1415926\n"
            "#define MAX_SIZE 100\n";

    tests[1] =
            "int addNumbers(int a, int b);\n"
            "void greetUser(char* name);\n"
            "float calculateCircleArea(float radius);\n";

    tests[2] =
            "const float PI_CONST = 3.1415926f;\n"
            "int globalCounter = 0;\n";

    tests[3] =
            "struct Point {\n"
            "    int x;\n"
            "    int y;\n"
            "};\n"
            "\n"
            "typedef struct Point Point;\n";

    tests[4] =
            "void test() {\n"
            "    int num1, num2, i;\n"
            "    float res, d;\n"
            "    char res2[50];\n"
            "    int array[10];\n"
            "    Point p1;\n"
            "}\n";

    tests[5] =
            "void test() {\n"
            "    printf(\"Enter a number: \");\n"
            "    scanf(\"%f\", &res);\n"
            "    printf(\"From input: %.2f\\n\", res);\n"
            "}\n";

    tests[6] =
            "void test() {\n"
            "    num1 = 12 / 2;\n"
            "    num2 = 15 % 4;\n"
            "    num1 = addNumbers(5, 3);\n"
            "}\n";
    tests[7] =
            "void test() {\n"
            "    switch(num1) {\n"
            "        case 1:\n"
            "        case 2:\n"
            "        case 3:\n"
            "        case 4:\n"
            "        case 5:\n"
            "            printf(\"Switch works\\n\");\n"
            "            break;\n"
            "        default:\n"
            "            printf(\"Switch no works\\n\");\n"
            "            break;\n"
            "    }\n"
            "}\n";
    tests[8] =
            "void test() {\n"
            "    if (5 % 3 > 0) {\n"
            "        printf(\"Yes,if 1\\n\");\n"
            "        printf(\"Yes,if 2\\n\");\n"
            "    } else {\n"
            "        printf(\"No,else 1\\n\");\n"
            "        printf(\"No,else 2\\n\");\n"
            "    }\n"
            "}\n";
    tests[9] =
            "void test() {\n"
            "    strcpy(res2, \"Hello world\");\n"
            "    num1 = 2;\n"
            "}\n";
    tests[10] =
            "void test() {\n"
            "    if (PI_CONST != num1) {\n"
            "        printf(\"Pim\\n\");\n"
            "        if (PI_CONST != num1) {\n"
            "            printf(\"Pam\\n\");\n"
            "        }\n"
            "    } else {\n"
            "        printf(\"Pum\\n\");\n"
            "    }\n"
            "}\n";
    tests[11] =
            "void test() {\n"
            "    for (i = 1; i <= 8; i++) {\n"
            "        printf(\"3\");\n"
            "    }\n"
            "}\n";
    tests[12] =
            "void test() {\n"
            "    while (num1 < 6) {\n"
            "        printf(\"Yes\");\n"
            "        num1 = num1 + 1;\n"
            "    }\n"
            "}\n";
    tests[13] =
            "void test() {\n"
            "    do {\n"
            "        printf(\"3\");\n"
            "        num1 = num1 + 1;\n"
            "    } while (num1 < 7);\n"
            "}\n";
    tests[14] =
            "void test() {\n"
            "    for (i = 0; i < 10; i++) {\n"
            "        array[i] = i * 2;\n"
            "    }\n"
            "}\n";
    tests[15] =
            "void test() {\n"
            "    p1.x = 10;\n"
            "    p1.y = 20;\n"
            "    int* ptr = &num1;\n"
            "    *ptr = *ptr + 3;\n"
            "}\n";
    tests[16] =
            "void test() {\n"
            "    num1 = num1 + 3;\n"
            "    num1 += 5;\n"
            "    num1++;\n"
            "}\n";
    tests[17] =
            "void test() {\n"
            "    if (num1 > 10 && num2 < 20 || !(num1 == 15)) {\n"
            "        printf(\"Complex condition works\\n\");\n"
            "    }\n"
            "}\n";
    tests[18] =
            "void test() {\n"
            "    num1 = num1 & 0xFF;\n"
            "    num2 = num1 | 0x0F;\n"
            "}\n";
    tests[19] =
            "void test() {\n"
            "    int result = (num1 > num2) ? num1 : num2;\n"
            "}\n";
    tests[20] =
            "int addNumbers(int a, int b) {\n"
            "    return a + b;\n"
            "}\n"
            "\n"
            "void greetUser(char* name) {\n"
            "    printf(\"Hello, %s!\\n\", name);\n"
            "}\n"
            "\n"
            "float calculateCircleArea(float radius) {\n"
            "    return PI_CONST * radius * radius;\n"
            "}\n";
    /*
  //  cout << "=== ORIGINAL CODE ===\n" << pascalCode << "\n\n";

//    // STEP 1: Parse Pascal -> Expression* (parseOnly - без выполнения)
//    Lexer lexer(pascalCode, PASCAL);
//    PascalParserToExpression parser(lexer, PASCAL);
//    parser.parseOnly();  // Только структура, без выполнения
//
//    const auto &exprs = parser.getExpressionsOnly();
//    cout << "Parsing: created " << exprs.size() << " expressions\n\n";
//
//    PascalToJSON exporter2;
//    std::string title =  parser.getTitle();
//
//    std::string mmd2 = exporter2.build(exprs,title);
//    //std::cout<<"Output of mmd2: \n"<<mmd2<<"\n";
//    // Экспорт блок-схемы по существующим Expression (Mermaid и JSON)
//    std::ofstream f("flowchart.json");
//    f << mmd2;
//    f.close();
//    cout << "Mermaid saved to flowchart.json\n\n";
//
//    // STEP 3: Import back from JSON and generate Pascal using PascalCodeGenerator
//    PascalCodeGenerator codeGenerator;
//    nlohmann::json jsonData = nlohmann::json::parse(mmd2);
//    std::string restoredPascal = codeGenerator.generatePascal(jsonData);
//
//    cout << "Import: restored Pascal code directly\n\n";
//
//    cout << "=== RESTORED CODE ===\n" << restoredPascal << "\n\n";
//    // Comparison
//    if (pascalCode == restoredPascal) {
//        cout << "SUCCESS: Code is identical!\n";
//    } else {
//        cout << "NOTE: Code differs (formatting may vary)\n";
//        cout << "Main structures should match.\n";
//    }
*/

    try {
        CParserToAST parser;
        auto ast = parser.parse(cCode);
        std::cout<<"TEST cCode"<<std::endl;
        std::cout<<cCode<<std::endl;
        AstToJsonConverter converter;
        nlohmann::json mmd3 = converter.convertProgram(ast);
        std::ofstream f2("flowchart2.json");
        f2 << mmd3.dump(4);
        f2.close();
//        for (int i = 0; i < 21; i++) {
//            if (i == 16) continue;
//            auto ast = parser.parse(tests[i]);
//            //ast->printTree();
//            if (ast) {
//                std::cout << "AST parsed successfully!\n";
//                std::cout << "AST Structure:\n";
//
//                AstVisualizer visualizer;
//                ast->accept(visualizer);
//            } else {
//                std::cout << "Failed to parse AST\n";
//            }
//        }


//        CppCodeGenerator generator;
//        nlohmann::json result = generator.convert(*ast);

        // std::cout << result.dump(2) << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }

    return 0;
}

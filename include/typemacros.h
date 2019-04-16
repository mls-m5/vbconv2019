/*
 * typemacros.h
 *
 *  Created on: 15 apr. 2019
 *      Author: Mattias Larsson Sköld
 */


// tn is values that should be a enum value and a corresponding code keyword

// tl is a value that should have a enum value but a custum string
// td is a value that should only have a enum but not any string
// ts is a value that have a enum and a string, but is not a keyword


// Types that is not associated to keywords:
ts(Literal)
ts(Numeric)
ts(Operator)
ts(BinaryOperator)
ts(BinaryExpression)
ts(Parenthesis)
ts(Group)
ts(Line)
ts(Block)
ts(BlockEnd)
ts(BlockBegin)
ts(FunctionBlock)

// Binary operations in falling precedence
// https://docs.microsoft.com/en-us/dotnet/visual-basic/language-reference/operators/operator-precedence
ts(Exponentiation)
ts(UnaryIdentityOrNegation) // ^
ts(MultiplicationOrDivision) // *, /
ts(IntegerDivision) // \ .
ts(ModulusOperation) // Mod
ts(AdditionOrSubtraction) // +, -
ts(StringConcatenation) // &
ts(ArithmeticBitShift) // <<, >>
ts(ComparisonOperation) // =, <>, <, <=, >, >=, Is, IsNot, Like, TypeOf
ts(Negation) // Not
ts(Conjunction) // And, AndAlso
ts(InclusiveDisjunction) // Or, OrElse
ts(ExclusiveDisjunction) // Xor

ts(AsClause)
ts(ComaList)
ts(SubStatement)
ts(EndStatement)
ts(ClassStatement)
ts(IfStatement)
ts(ElIfStatement)
ts(ElseStatement)

ts(MethodCall)

tn(As)
tn(Long)
tn(Sub)
tn(End)
tn(Function)
tn(Class)

td(AccessSpecifiersBegin)
tn(Public)
tn(Private)
tn(Protected)
td(AccessSpecifiersEnd)



tn(Dim)
tn(Redim)
tn(Set)
tn(Get)


tn(While)
tn(Wend)
tn(Do)
tn(For)
tn(Next)
tn(Loop)
tn(New)
tn(Return)
tn(If)
tn(Then)
tn(Elif)
tn(Else)
tn(With)
tn(To)
tn(True)
tn(False)
tn(Mod)


td(TypesBegin)
tn(Single)
tn(Double)
tn(Byte)
tn(Integer)
tn(Boolean)
td(TypesEnd)


tn(ByVal)


tn(Module)
tn(Error)
tn(Enum)
tn(Default)
tn(Option)
tn(Of)


tn(Not)
tn(And)
tn(Or)
tn(Xor)
tl(Coma, ",")
tl(Asterisks, "*")
tl(Slash, "/")
tl(Backslash, "\\")
tl(Plus, "+")
tl(Minus, "-")
tl(Exp, "^")
tl(SingleQuote, "'")

//Comparisons
tl(Equal, "=")
tl(NotEqual, "<>")
tl(More, ">")
tl(MoreOrEqual, ">=")
tl(Less, "<")
tl(LessOrEqual, "<=")
tn(Is)
tn(IsNot)
tn(Like)

tn(TypeOf)


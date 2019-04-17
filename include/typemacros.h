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
ts(Root)
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

ts(Heading)

ts(PropertyAccessor)
ts(MemberAccessor)

// Binary operations in falling precedence
// https://docs.microsoft.com/en-us/dotnet/visual-basic/language-reference/operators/operator-precedence
ts(Exponentiation) // ^
ts(UnaryIdentityOrNegation) // +, - (prefix)
ts(MultiplicationOrDivision) // *, /
ts(IntegerDivision) // \ .
ts(ModulusOperation) // Mod
ts(AdditionOrSubtraction) // +, -
ts(StringConcatenation) // &
ts(ArithmeticBitShift) // <<, >>
ts(ComparisonOperation) // =, <>, <, <=, >, >=, Is, IsNot, Like, TypeOf
ts(LogicalNegation) // Not
ts(Conjunction) // And, AndAlso
ts(InclusiveDisjunction) // Or, OrElse
ts(ExclusiveDisjunction) // Xor

ts(AsClause)
ts(TypeCharacterClause)
ts(ComaList)
ts(SubStatement)
ts(EndStatement)
ts(ClassStatement)
ts(IfStatement)
ts(InlineIfStatement)
ts(ElIfStatement)
ts(ElseStatement)
ts(WithStatement)
ts(FunctionCallOrPropertyAccessor)
ts(CallStatement)
ts(DimStatement)
ts(OptionStatement)
ts(Assignment)
ts(SetStatement)

ts(MethodCall)

tn(As)
tn(Sub)
tn(Begin)
tn(End)
tn(Function)
tn(Class)
tn(Attribute)

tn(Public)
tn(Private)
tn(Protected)

ts(AccessSpecifier)

tn(Dim)
tn(Redim)
tn(Set)
tn(Get)
tn(Call)


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


td(TypesBegin)
tn(Single)
tn(Long)
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

tl(Percentage, "%")
tl(Hash, "#")
tl(Exclamation, "!")
tl(At, "@")
tl(Et, "&")
tl(Dollar, "$")
tl(SingleQuote, "'")
tl(LeftParenthesis, "(")
tl(RightParenthesis, ")")
tn(TypeOf)

td(BinaryOperatorsBegin)

tn(Not)
tn(And)
tn(AndAlso)
tn(Or)
tn(OrElse)
tn(Xor)
tn(Mod)
tl(Coma, ",")
tl(Dot, ".")
tl(Asterisks, "*")
tl(Slash, "/")
tl(Backslash, "\\")
tl(Plus, "+")
tl(Minus, "-")
tl(Exp, "^")

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

td(BinaryOperatorsEnd)

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
//ts(MemberAccessor)

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
ts(FunctionCallOrPropertyAccessor)
ts(CommaList)

ts(AsClause)
ts(DefaultAsClause)
ts(TypeCharacterClause)
ts(DoubleComma)
ts(SubStatement)
ts(FunctionStatement)
ts(EndStatement)
ts(ClassStatement)
ts(IfStatement)
ts(InlineIfStatement)
ts(InlineIfElseStatement)
ts(ElseIfStatement)
//ts(ElseStatement)
ts(ForLoop)
ts(NextStatement)
ts(LoopWhileStatement)
ts(WithStatement)
ts(CallStatement)
ts(DimStatement)
ts(RedimStatement)
ts(OptionStatement)
ts(Assignment)
ts(SetStatement)
ts(RefTypeStatement)
ts(OptionalStatement)
ts(NewStatement)
ts(FileNumberStatement)
ts(OpenStatement)
ts(AttributeStatement)
ts(EnumStatement)
ts(ExitStatement)
ts(DeclareStatement)
ts(SelectCaseStatement)
ts(CaseStatement)
ts(ConstStatement)
ts(TypeStatement)

ts(MethodCall)

tn(As)
tn(Sub)
tn(Begin)
tn(End)
tn(Function)
tn(Class)
tn(Attribute)
tn(Declare)
tn(Lib)
tn(Const)

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
tn(Step)
tn(New)
tn(Return)
tn(If)
tn(Then)
tn(Elseif)
tn(Else)
tn(With)
tn(To)
tn(True)
tn(False)
tn(Open)
tn(Binary)
tn(Select)
tn(Case)


td(TypesBegin)
tn(Single)
tn(Long)
tn(Double)
tn(Byte)
tn(Integer)
tn(Boolean)
tn(String)
td(TypesEnd)


tn(ByVal)
tn(ByRef)
tn(Optional)

tn(Module)
tn(Error)
tn(Enum)
tn(Default)
tn(Option)
tn(Of)
tn(Me)
tn(Exit)

tl(TypeKeyword, "Type") // "Type" collides with Token::Type
tl(Percentage, "%")
tl(Hash, "#")
tl(Exclamation, "!")
tl(At, "@")
tl(Dollar, "$")
tl(SingleQuote, "'")
tl(LeftParenthesis, "(")
tl(RightParenthesis, ")")
tn(TypeOf)

td(BinaryOperatorsBegin)

tl(Et, "&")
tn(Not)
tn(And)
tn(AndAlso)
tn(Or)
tn(OrElse)
tn(Xor)
tn(Mod)
tl(Comma, ",")
tl(Dot, ".")
tl(Asterisks, "*")
tl(Slash, "/")
tl(Backslash, "\\")
tl(Plus, "+")
tl(Minus, "-")
tl(Exp, "^")

tl(ShiftLeft, "<<")
tl(ShiftRight, ">>")

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


ts(CVariableDeclaration)

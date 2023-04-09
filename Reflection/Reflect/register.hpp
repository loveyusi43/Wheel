#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <string>
#include <functional>

#include "factory.h"

class Register
{
public:
	Register(const std::string& name, CreateObject method)
	{
		Factory::Instance()->RegisterClass(name, method);
	}

	Register(const std::string& class_name, const std::string& field, const std::string& type, size_t offset)
	{
		Factory::Instance()->RegisterField(class_name, field, type, offset);
	}

	Register(const std::string& class_name, const std::string& method_name, uintptr_t method_ptr)
	{
		Factory* factory = Factory::Instance();
		factory->RegisterClassMethod(class_name, method_name, method_ptr);
	}
};

#define REGISTER_CLASS(ClassName)\
	Object* Create##ClassName()\
	{\
		Object* obj = new ClassName();\
		obj->SetClassName(#ClassName);\
		return obj;\
	}\
	Register Register##ClassName(#ClassName, &Create##ClassName)


#define REGISTER_CLASS_FIELD(ClassName, FieldName, FieldType)\
	Register Register##ClassName##FieldName(#ClassName, #FieldName, #FieldType, offsetof(ClassName, FieldName))


#define REGISTER_CLASS_METHOD(ClassName, MethodName)\
	std::function<int(ClassName*, int)> ClassName##MethodName##method = &ClassName::MethodName;\
	Register Register##ClassName##MethodName(#ClassName, #MethodName, (uintptr_t)&(ClassName##MethodName##method))


#endif   //  REGISTER_H
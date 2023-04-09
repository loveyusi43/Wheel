#ifndef FACTORY
#define FACTORY

#include <unordered_map>
#include <string>
#include <vector>

#include "field.hpp"
#include "method.hpp"

class Object
{
public:
	Object() = default;
	virtual ~Object() = default;

	size_t GetFieldCount() const;

	template<typename T>
	T& Get(const std::string& field_name);

	template<typename T>
	void Set(const std::string& field_name, const T& value);

	virtual void Show() = 0;

	void SetClassName(const std::string& class_name);

	int Call(const std::string& method_name, int num);

protected:
	const std::string& GetClassName() const;
	Field* GetFiled(size_t pos) const;
	Field* GetField(const std::string& field_name) const;

private:
	std::string class_name_;
};

/************************************************************************************************************/

using CreateObject = Object* (*)();


class Factory
{
public:
	void RegisterClass(const std::string& class_name, CreateObject method);
	Object* CreateClass(const std::string& class_name);

	static Factory* Instance();

	void RegisterField(const std::string& class_name, const std::string& field_name, const std::string& type, size_t offset);
	size_t GetFieldCount(const std::string& class_name);
	Field* GetField(const std::string& class_name, size_t pos);
	Field* GetField(const std::string& class_name, const std::string& field_name);

	void RegisterClassMethod(const std::string& class_name, const std::string& method_name, uintptr_t ptr);
	size_t GetMethodCount(const std::string&);
	Method* GetClassMethod(const std::string& class_name, size_t pos);
	Method* GetClassMethod(const std::string& class_name, const std::string& method_name);

private:
	Factory() = default;
	~Factory() = default;

protected:
	std::unordered_map<std::string, CreateObject> class_map_;
	std::unordered_map<std::string, std::vector<Field*>> fields_map_;
	std::unordered_map<std::string, std::vector<Method*>> methods_map_;
	static Factory* instance_;
};

/******************************************************************************************/

template<typename T>
T& Object::Get(const std::string& field_name)
{
	Factory* factory = Factory::Instance();
	const Field* field = factory->GetField(class_name_, field_name);
	const size_t offset = field->Offset();
	return *((T*)((unsigned char*)this + offset));
}

template<typename T>
void Object::Set(const std::string& field_name, const T& value)
{
	Factory* factory = Factory::Instance();
	const Field* field = factory->GetField(class_name_, field_name);
	const size_t offset = field->Offset();
	*((T*)((unsigned char*)this + offset)) = value;
}

#endif // FACTORY
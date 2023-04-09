#include <mutex>
#include <functional>

#include "factory.h"

void Object::SetClassName(const std::string& class_name)
{
	class_name_ = class_name;
}

const std::string& Object::GetClassName() const
{
	return class_name_;
}

size_t Object::GetFieldCount() const
{
	Factory* factory = Factory::Instance();
	return factory->GetFieldCount(class_name_);
}

Field* Object::GetFiled(size_t pos) const
{
	return Factory::Instance()->GetField(class_name_, pos);
}

Field* Object::GetField(const std::string& field_name) const
{
	return Factory::Instance()->GetField(class_name_, field_name);
}

int Object::Call(const std::string& method_name, int num)
{
	Factory* factory = Factory::Instance();
	const Method* method = factory->GetClassMethod(class_name_, method_name);
	const uintptr_t func_ptr = method->MethodPtr();
	using ClassMethod = std::function<int(decltype(this), int)>;
	return (*(ClassMethod*)func_ptr)(this, num);
}


/***********************************************************************************************************/

Factory* Factory::instance_ = nullptr;

Factory* Factory::Instance()
{
	if (nullptr == instance_)
	{
		static std::mutex mtx;
		mtx.lock();
		if (nullptr == instance_)
		{
			instance_ = new Factory();
		}
		mtx.unlock();
	}
	return instance_;
}

void Factory::RegisterClass(const std::string& class_name, CreateObject method)
{
	class_map_[class_name] = method;
}

Object* Factory::CreateClass(const std::string& class_name)
{
	const std::unordered_map<std::string, CreateObject>::iterator it = class_map_.find(class_name);
	if (it == class_map_.end())
	{
		return nullptr;
	}
	return it->second();
}

void Factory::RegisterField(const std::string& class_name, const std::string& field_name, const std::string& type, size_t offset)
{
	fields_map_[class_name].push_back(new Field(field_name, type, offset));
}

size_t Factory::GetFieldCount(const std::string& class_name)
{
	return fields_map_[class_name].size();
}

Field* Factory::GetField(const std::string& class_name, size_t pos)
{
	if (pos >= fields_map_[class_name].size())
	{
		return nullptr;
	}

	return fields_map_[class_name][pos];
}

Field* Factory::GetField(const std::string& class_name, const std::string& field_name)
{
	for (Field*& field : fields_map_[class_name])
	{
		if (field->Name() == field_name)
		{
			return field;
		}
	}
	return nullptr;
}

void Factory::RegisterClassMethod(const std::string& class_name, const std::string& method_name, uintptr_t ptr)
{
	methods_map_[class_name].push_back(new Method(method_name, ptr));
}

size_t Factory::GetMethodCount(const std::string& class_name)
{
	return methods_map_[class_name].size();
}

Method* Factory::GetClassMethod(const std::string& class_name, size_t pos)
{
	if (pos >= methods_map_[class_name].size())
	{
		return nullptr;
	}
	return methods_map_[class_name][pos];
}

Method* Factory::GetClassMethod(const std::string& class_name, const std::string& method_name)
{
	std::vector<Method*> methods = methods_map_[class_name];
	for (Method*& m : methods)
	{
		if (m->MethodName() == method_name)
		{
			return m;
		}
	}
	return nullptr;
}
#ifndef IBK_DependencyObjectH
#define IBK_DependencyObjectH

#include <algorithm>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace IBK {

/*! A light-weight object that holds dependencies to other objects and can be sorted with a topology sort
	via member function sort(). For more efficient sorting, consider using Zeppelin lib.

	\code

		// 1. Create your dependency objects for all objects you want to sort.

		DependencyObject<WorkerStep> obj1(m_workerStep[0]);
		DependencyObject<WorkerStep> obj2(m_workerStep[1]);

		// 2. Add interdependencies by adding pointers to m_dependencies variable.

		obj2.m_dependencies.push_back(&obj1);

		// 3. Create vector with objects to sort

		std::vector<DependencyObject*> objs { &obj1, &obj2 };

		// 4. sort

		DependencyObject<WorkerStep>::sort(objs);
	\endcode

	Here is a full example:
	\code
		// DependencySort Test

		// objects to sort
		std::vector<std::string> strs = {"a", "b", "c", "d", "e"};

		std::vector<IBK::DependencyObject<std::string> *> deps;
		for (const std::string & s : strs)
			deps.push_back(new IBK::DependencyObject<std::string>(s));

		deps[0]->m_dependencies.push_back(deps[1]); // a -> b : "a" hängt ab von "b"
		deps[1]->m_dependencies.push_back(deps[3]); // b -> d
		deps[0]->m_dependencies.push_back(deps[2]); // a -> c
		deps[4]->m_dependencies.push_back(deps[1]); // e -> a
		deps[2]->m_dependencies.push_back(deps[3]); // c -> d

		// expected sort order (evaluation order): "d, b, c, a, e" or "d, c, b, a, e"

		IBK::DependencyObject<std::string>::sort(deps);
		// deps[0]->sort(deps);

		// dump out sort result
		for (auto & dobj : deps) {
			IBK::IBK_Message(IBK::FormatString("%1\n").arg(dobj->m_payload));
			delete dobj;
		}
	\endcode
*/
template<typename T>
class DependencyObject {
public:
	DependencyObject(const T & payload) : m_payload(payload){}

	/*! Does a topological sort. Returns false in case of circular dependencies.
		Objects without dependencies are front, objects with dependencies follow accordingly later in vector.
		\return Returns false
	*/
	static bool sort(std::vector<DependencyObject<T> *> &objects);

	std::vector<DependencyObject*>	m_dependencies;
	const T							&m_payload;

private:
	/*! Does a recursive topological sort. This is a helper function for sort(). */
	static bool topologicalSort(DependencyObject* obj,
								std::unordered_set<DependencyObject*>& visited,
								std::unordered_set<DependencyObject*>& inStack,
								std::stack<DependencyObject*>& result)
	{
		// is the processed object already in stack (i.e. somewhere up the call stack already processed?)
		if (inStack.count(obj)) {
			// std::cerr << "Zyklische Abhängigkeit entdeckt!" << std::endl;
			return false; // cyclic dependency
		}

		if (visited.count(obj))
			return true;  // already visited(i.e. was in the dependency of some previously checked object already)

		// mark as visited and push to stack
		visited.insert(obj);
		inStack.insert(obj);

		// now process all dependencies
		for (auto dep : obj->m_dependencies)
			if (!topologicalSort(dep, visited, inStack, result))
				return false;

		// remove from stack again
		inStack.erase(obj);

		// put object to stack with dependent objects, _after_ its own dependencies
		result.push(obj);
		return true;
	}
};


template<typename T>
inline bool DependencyObject<T>::sort(std::vector<DependencyObject<T>*> &objects) {
	// Topologische Sortierung der Objekte
	std::unordered_set<DependencyObject*> visited;
	std::unordered_set<DependencyObject*> inStack;
	std::stack<DependencyObject*> sortedStack;

	for (auto obj : objects)
		if (!topologicalSort(obj, visited, inStack, sortedStack))
			return false;

	// copy result back to objects vector
	objects.clear();
	while (!sortedStack.empty()) {
		objects.push_back(sortedStack.top());
		sortedStack.pop();
	}
	// and reverse vector, so that objects without dependencies are front
	std::reverse(objects.begin(), objects.end());
	return true;
}

} // namespace IBK

#endif // IBK_DependencyObjectH

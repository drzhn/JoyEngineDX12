#ifndef TREE_STORAGE_H
#define TREE_STORAGE_H
#include <cstdint>
#include <unordered_map>

namespace JoyEngine
{
	template <typename T>
	class TreeEntry
	{
	public :
		[[nodiscard]] T* GetParent() { return m_parent; }
		[[nodiscard]] T* GetNextSibling() { return m_nextSibling; }
		[[nodiscard]] T* GetFirstChild() { return m_firstChild; }
	protected:
		T* m_parent = nullptr;
		T* m_nextSibling = nullptr;
		T* m_firstChild = nullptr;
	};

	template <typename T, uint32_t Size> requires std::is_base_of_v<TreeEntry<T>, T>
	class TreeStorage
	{
	public:
		template <typename ObjT, typename... Args> requires std::is_base_of_v<T, ObjT>
		ObjT* Create(Args&&... args)
		{
			std::unique_ptr<ObjT> up = std::make_unique<ObjT>(std::forward<Args>(args)...);
			ObjT* ptr = up.get();
			m_storage.insert({ptr, std::move(up)});
			if (m_root == nullptr)
			{
				m_root = ptr;
			}
			return ptr;
		}

		void Clear()
		{
			m_storage.clear();
		}

		void Remove(T* ptr)
		{
			// TODO
		}

	private:
		T* m_root = nullptr;

		std::unordered_map<T*, std::unique_ptr<T>> m_storage;
		// TODO allocate entries in custom allocator
	};
}
#endif // TREE_STORAGE_H

#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

namespace JoyEngine {
	class Serializable {
	public:
		Serializable() = default;

		virtual ~Serializable() = default;

		//virtual void Serialize() = 0;
	};
}

#endif //SERIALIZABLE_H

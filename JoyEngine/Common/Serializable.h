#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include "JoyObject.h"

namespace JoyEngine
{
	class Serializable : public JoyObject
	{
		DECLARE_JOY_OBJECT(Serializable, JoyObject);
	public:
		Serializable() = default;

		virtual ~Serializable() = default;

		//virtual void Serialize() = 0;
	};
}

#endif //SERIALIZABLE_H

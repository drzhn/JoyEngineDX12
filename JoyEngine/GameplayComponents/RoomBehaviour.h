#ifndef ROOM_BEHAVIOUR_H
#define ROOM_BEHAVIOUR_H

#include "Common/Color.h"
#include "Common/Serialization.h"
#include "Components/Component.h"

namespace JoyEngine {

	DECLARE_CLASS(RoomBehaviour)

    class RoomBehaviour : public Component {

        DECLARE_CLASS_NAME(RoomBehaviour);

        REFLECT_FIELD(float, m_speed);
        REFLECT_FIELD(Color, m_color);

    public :

        explicit RoomBehaviour(GameObject& go);

        void Enable() final;

        void Disable() final;

        void Update() final;
    };


}

#endif //ROOM_BEHAVIOUR_H

#ifndef ROOM_BEHAVIOUR_H
#define ROOM_BEHAVIOUR_H

#include "Common/Serialization.h"
#include "Components/Component.h"

namespace JoyEngine {

    class RoomBehaviour : public Component {

        DECLARE_CLASS_NAME(RoomBehaviour)

        REFLECT_FIELD(float, m_speed);
        REFLECT_FIELD(Color, m_color);


    public :

        void Enable() final;

        void Disable() final;

        void Update() final;
    };

}

#endif //ROOM_BEHAVIOUR_H

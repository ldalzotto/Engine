#pragma  once

namespace v2
{
	struct PrimitiveSerializedTypes
	{
		enum class Type
		{
			UNDEFINED = 0,
			FLOAT32 = UNDEFINED + 1,
			FLOAT32_2 = FLOAT32 + 1,
			FLOAT32_3 = FLOAT32_2 + 1,
			FLOAT32_4 = FLOAT32_3 + 1,
		};

		inline static uimax get_size(const Type p_type)
		{
			switch(p_type)
			{
			case Type::FLOAT32:
				return sizeof(float32);
			case Type::FLOAT32_2:
				return sizeof(float32) * 2;
			case Type::FLOAT32_3:
				return sizeof(float32) * 3;
			case Type::FLOAT32_4:
				return sizeof(float32) * 4;
			default:
				abort();
			}
		};
	};

};

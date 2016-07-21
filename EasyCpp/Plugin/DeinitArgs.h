#pragma once
#include <string>
#include "../DllExport.h"

namespace EasyCpp
{
	namespace Plugin
	{
		class DLL_EXPORT DeinitArgs
		{
		public:
			enum class Reason
			{
				IN_USE = 0
			};
			void cancel(Reason r, const std::string& reason = "");

			bool isCanceled();
			Reason getCancelReason();
			std::string getCancelString();
		private:
			bool _cancel = false;
			Reason _cancel_reason;
			std::string _cancel_reason_str;
		};
	}
}
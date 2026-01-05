#pragma once

#include <memory>

namespace Tokens
{
	class TokenSource;
	class LifetimeToken
	{
	private:
		bool Valid = true;

		friend TokenSource;
		void Invalidate() { Valid = false; };
	public:
		bool IsValid() const { return Valid; }
		LifetimeToken() = default;
		
		// noncopyable
		LifetimeToken(const LifetimeToken&) = delete;
		LifetimeToken& operator = (const LifetimeToken&) = delete;
	};

	using LifetimeTokenPtr = std::shared_ptr<LifetimeToken>;

	class TokenSource
	{
	private:
		LifetimeTokenPtr RootToken;
	public:
		TokenSource()
		{
			RootToken = std::make_shared<LifetimeToken>();
		}

		~TokenSource()
		{
			RootToken->Invalidate();
		}

		// noncopyable
		TokenSource(const TokenSource&) = delete;
		TokenSource& operator = (const TokenSource&) = delete;

		LifetimeTokenPtr GetToken() const
		{
			return RootToken;
		}
	};
}
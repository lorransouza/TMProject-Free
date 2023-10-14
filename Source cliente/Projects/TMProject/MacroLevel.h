#pragma once

namespace Macro
{
	class MacroLevel
	{
	public:
		virtual ~MacroLevel() = default;

		virtual void UseItem(const int posX, const int posY, const int level) = 0;

		virtual void DoMove(const int posX, const int posY) = 0;
	};

	class Water_N : public MacroLevel
	{
	public:
		Water_N();
		~Water_N();

		void UseItem(const int posX, const int posY, const int level) override;

		void DoMove(const int posX, const int posY) override;

	private:
		int firstScroll = 3173;

		int lastScroll = 3181;


	};

	class Water_M : public MacroLevel
	{
	public:
		Water_M();
		~Water_M();

		void UseItem(const int posX, const int posY, const int level) override;

		void DoMove(const int posX, const int posY) override;

	private:
		int firstScroll = 777;

		int lastScroll = 785;
	};

	class Water_A : public MacroLevel
	{
	public:
		Water_A();
		~Water_A();

		void UseItem(const int posX, const int posY, const int level) override;

		void DoMove(const int posX, const int posY) override;

	private:
		int firstScroll = 3182;

		int lastScroll = 3190;
	};
}

#ifndef __LOOPSTRENGTHREDUCTION_HH__
#define __LOOPSTRENGTHREDUCTION_HH__

#include "loopinductionderived.hh"
#include <iostream>

// StrengthReductionPlan represents the transformations needed for one loop
class StrengthReductionPlan {
public:
	struct ReplacementIV {
		struct TempMap {
			int headerLabel = -1;
			int oldTempNum = -1;
			int basicTempNum = -1;
			int newPhiTemp = -1;
			int newBackedgeTemp = -1;
		};

		struct InitExpr {
			int initTempNum = -1;
			int basicCoeffConst = 1;
			int basicCoeffInvTempNum = -1;
			int constant = 0;
			int invOffsetTempNum = -1;
			int invOffsetScale = 1;
			bool invOffsetNegative = false;
			bool sourceAfterBasicUpdate = false;
			int basicStepTempNum = -1;
			int basicStepValue = 0;
		};

		struct InitTemps {
			int newInitTemp = -1;
			int newInitAdjustedSourceTemp = -1;
			int newInitIntermediateTemp = -1;
			int newInitScaleTemp = -1;
			int newInitOffsetTemp = -1;
		};

		struct StepExpr {
			int stepIncrementValue = 0;
			int stepIncrementTempNum = -1;
			bool stepIncrementNegative = false;
			int stepTempScaleFactor = 1;
			int newStepTemp = -1;
			int stepSourceTempNum = -1;      // Primary source temp for step preparation
			int stepSecondaryTempNum = -1;   // Optional secondary source when step needs temp*temp
			int stepMulTemp = -1;            // Optional intermediate temp for (temp*temp)*const
		};

		struct Placement {
			int initLabel = -1;
			int backedgeLabel = -1;
		};

		size_t sourceOrder = 0;

		TempMap map;
		InitExpr initExpr;
		InitTemps initTemps;
		StepExpr stepExpr;
		Placement placement;
	};

	// Mapping from original derived IV temp to replacement temp
	map<int, int> tempReplacement;
	
	// List of statements to remove from the original code
	set<quad::QuadStm*> stmtsToRemove;
	
	// Map of new PHI statements to add to loop header
	// PHI temp -> (init value temp, backedge value temp)
	map<int, pair<int, int>> phiStmtsToAdd;
	
	// Map of new update statements to add to loop body
	// temp -> (new update statement info)
	map<int, pair<int, int>> updateStmtsToAdd; // temp -> (scale factor, offset)

	// Concrete replacement records used for statement insertion in apply stage.
	vector<ReplacementIV> replacements;
	
	void print() const {
		if (replacements.empty()) {
			std::cout << "=== NO STRENGTH REDUCTIONS PLANNED ===" << std::endl;
			return;
		}

		auto printSignedConst = [](int value, bool& printedAny) {
			if (value == 0) {
				return;
			}
			if (!printedAny) {
				std::cout << value;
				printedAny = true;
				return;
			}
			if (value > 0) {
				std::cout << " + " << value;
			} else {
				std::cout << " - " << (-value);
			}
		};

		auto printSignedTemp = [](int tempNum, bool negative, int scale, bool& printedAny) {
			if (tempNum == -1) {
				return;
			}
			if (!printedAny) {
				if (negative) {
					std::cout << "-";
				}
			} else {
				std::cout << (negative ? " - " : " + ");
			}
			if (scale != 1) {
				std::cout << scale << "*";
			}
			std::cout << "t" << tempNum;
			printedAny = true;
		};
		
		std::cout << "=== STRENGTH REDUCTION PLAN ===" << std::endl;
		std::cout << "Total replacements: " << replacements.size() << std::endl;
		for (size_t i = 0; i < replacements.size(); ++i) {
			const ReplacementIV& repl = replacements[i];
			std::cout << "Replacement " << (i + 1) << ":" << std::endl;
			std::cout << "  Original IV: t" << repl.map.oldTempNum << std::endl;
			std::cout << "  New PHI temp: t" << repl.map.newPhiTemp << std::endl;
			std::cout << "  New backedge temp: t" << repl.map.newBackedgeTemp << std::endl;
			std::cout << "  Loop header: L" << repl.map.headerLabel << std::endl;
			std::cout << "  Source order: ";
			if (repl.sourceOrder != static_cast<size_t>(-1)) {
				std::cout << repl.sourceOrder << std::endl;
			} else {
				std::cout << "unknown" << std::endl;
			}
			std::cout << "  Init adjusted for update order: "
			          << (repl.initExpr.sourceAfterBasicUpdate ? "yes" : "no") << std::endl;
			std::cout << "  Init expr: ";

			bool printedInit = false;
			if (repl.initExpr.basicCoeffConst != 0) {
				if (repl.initExpr.basicCoeffConst < 0) {
					std::cout << "-";
				}
				int absCoeff = repl.initExpr.basicCoeffConst >= 0
					? repl.initExpr.basicCoeffConst
					: -repl.initExpr.basicCoeffConst;
				if (absCoeff != 1) {
					std::cout << absCoeff << "*";
				}
				if (repl.initExpr.basicCoeffInvTempNum != -1) {
					std::cout << "t" << repl.initExpr.basicCoeffInvTempNum << "*";
				}
				std::cout << "t" << repl.map.basicTempNum;
				printedInit = true;
			}

			printSignedTemp(
				repl.initExpr.invOffsetTempNum,
				repl.initExpr.invOffsetNegative,
				repl.initExpr.invOffsetScale,
				printedInit
			);
			printSignedConst(repl.initExpr.constant, printedInit);

			if (!printedInit) {
				std::cout << "0";
			}
			std::cout << std::endl;

			std::cout << "  Step expr: ";
			if (repl.stepExpr.stepIncrementTempNum != -1) {
				std::cout << (repl.stepExpr.stepIncrementNegative ? "-" : "+")
				          << "t" << repl.stepExpr.stepIncrementTempNum;
			} else {
				std::cout << repl.stepExpr.stepIncrementValue;
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
};

// Generate strength reduction plan for a loop
// Returns a plan that can be applied to the function
StrengthReductionPlan generateStrengthReductionPlan(
	quad::QuadFuncDecl* func,
	const map<int, vector<DerivedInductionVar>>& derivedIVsByHeader,
	const map<int, vector<BasicInductionVar>>& basicIVsByHeader,
	LoopHeaderMap* loopHeaderMap
);

// Apply strength reduction transformations to the function
quad::QuadFuncDecl* applyStrengthReduction(
	quad::QuadFuncDecl* func,
	const StrengthReductionPlan& plan
);

#endif

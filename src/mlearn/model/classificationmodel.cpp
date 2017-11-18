/**
 * @file model/classificationmodel.cpp
 *
 * Implementation of the classification model.
 */
#include <iomanip>
#include "mlearn/model/classificationmodel.h"
#include "mlearn/util/logger.h"
#include "mlearn/util/timer.h"

namespace ML {

/**
 * Construct a classification model.
 *
 * @param feature
 * @param classifier
 */
ClassificationModel::ClassificationModel(FeatureLayer *feature, ClassifierLayer *classifier)
{
	// initialize layers
	this->_feature = feature;
	this->_classifier = classifier;

	// initialize stats
	this->_stats.error_rate = 0.0f;
	this->_stats.train_time = 0.0f;
	this->_stats.predict_time = 0.0f;

	// log hyperparameters
	log(LL_VERBOSE, "Hyperparameters");

	this->_feature->print();
	this->_classifier->print();

	log(LL_VERBOSE, "");
}

/**
 * Save a model to a file.
 *
 * @param path
 */
void ClassificationModel::save(const std::string& path)
{
	std::ofstream file(path, std::ofstream::out);

	this->_train_set.save(file);
	this->_mean.save(file);
	this->_feature->save(file);
	this->_P.save(file);

	file.close();
}

/**
 * Load a model from a file.
 *
 * @param path
 */
void ClassificationModel::load(const std::string& path)
{
	std::ifstream file(path, std::ifstream::in);

	this->_train_set.load(file);
	this->_mean.load(file);
	this->_feature->load(file);
	this->_P.load(file);

	file.close();
}

/**
 * Perform training on a training set.
 *
 * @param train_set
 */
void ClassificationModel::train(const Dataset& train_set)
{
	Timer::push("Training");

	this->_train_set = train_set;

	log(LL_VERBOSE, "Training set: %d samples, %d classes",
		train_set.entries().size(),
		train_set.labels().size());

	// get data matrix X
	Matrix X = train_set.load_data();

	// subtract mean from X
	this->_mean = X.mean_column();

	X.subtract_columns(this->_mean);

	// project X into feature space
	this->_feature->compute(X, this->_train_set.entries(), this->_train_set.labels().size());
	this->_P = this->_feature->project(X);

	// record training time
	this->_stats.train_time = Timer::pop();

	log(LL_VERBOSE, "");
}

/**
 * Perform recognition on a test set.
 *
 * @param test_set
 */
std::vector<DataLabel> ClassificationModel::predict(const Dataset& test_set)
{
	Timer::push("Prediction");

	log(LL_VERBOSE, "Test set: %d samples, %d classes",
		test_set.entries().size(),
		test_set.labels().size());

	// compute projected test images
	Matrix X_test = test_set.load_data();
	X_test.subtract_columns(this->_mean);

	Matrix P_test = this->_feature->project(X_test);

	// compute predicted labels
	std::vector<DataLabel> Y_pred = this->_classifier->predict(
		this->_P,
		this->_train_set.entries(),
		this->_train_set.labels(),
		P_test
	);

	// record prediction time
	this->_stats.predict_time = Timer::pop();

	log(LL_VERBOSE, "");

	return Y_pred;
}

/**
 * Validate a set of predicted labels against the ground truth.
 *
 * @param test_set
 * @param Y_pred
 */
void ClassificationModel::validate(const Dataset& test_set, const std::vector<DataLabel>& Y_pred)
{
	int num_errors = 0;

	for ( size_t i = 0; i < test_set.entries().size(); i++ ) {
		if ( Y_pred[i] != test_set.entries()[i].label ) {
			num_errors++;
		}
	}

	this->_stats.error_rate = (float) num_errors / test_set.entries().size();
}

/**
 * Print prediction results of a model.
 *
 * @param test_set
 * @param Y_pred
 */
void ClassificationModel::print_results(const Dataset& test_set, const std::vector<DataLabel>& Y_pred) const
{
	log(LL_VERBOSE, "Results");

	for ( size_t i = 0; i < test_set.entries().size(); i++ ) {
		const DataLabel& y_pred = Y_pred[i];
		const DataEntry& entry = test_set.entries()[i];

		const char *s = (y_pred != entry.label)
			? "(!)"
			: "";

		log(LL_VERBOSE, "%-12s -> %-4s %s", entry.name.c_str(), y_pred.c_str(), s);
	}

	log(LL_VERBOSE, "Error rate: %.3f", this->_stats.error_rate);
	log(LL_VERBOSE, "");
}

/**
 * Print a model's performance and accuracy statistics.
 */
void ClassificationModel::print_stats() const
{
	std::cout
		<< std::setw(12) << std::setprecision(3) << this->_stats.error_rate
		<< std::setw(12) << std::setprecision(3) << this->_stats.train_time
		<< std::setw(12) << std::setprecision(3) << this->_stats.predict_time
		<< "\n";
}

}
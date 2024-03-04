import argparse
import json
import os

import numpy as np
import tensorflow as tf

SAMPLE = 64
DIM = 8
OUTPUTS = 5


def model(x_train, y_train, x_test, y_test, batch_size, epochs, learning_rate):
    model = tf.keras.models.Sequential()

    model.add(tf.keras.layers.Conv2D(filters=16, kernel_size=(3, 3), strides=(
        1, 1), padding="same", name='block1_conv1', input_shape=(SAMPLE, DIM, 1)))
    model.add(tf.keras.layers.BatchNormalization(name='block1_bn'))
    model.add(tf.keras.layers.Activation('relu', name='block1_act'))
    model.add(tf.keras.layers.MaxPooling2D(
        padding='same', pool_size=(2, 2), name='block1_pool'))

    model.add(tf.keras.layers.Conv2D(filters=32, kernel_size=(3, 1),
              strides=(1, 1), padding="same", name='block2_conv1'))
    model.add(tf.keras.layers.BatchNormalization(name='block2_bn'))
    model.add(tf.keras.layers.Activation('relu', name='block2_act'))

    model.add(tf.keras.layers.Conv2D(filters=32, kernel_size=(3, 1),
              strides=(1, 1), padding="same", name='block3_conv1'))
    model.add(tf.keras.layers.BatchNormalization(name='block3_bn'))
    model.add(tf.keras.layers.Activation('relu', name='block3_act'))
    model.add(tf.keras.layers.MaxPooling2D(
        padding='same', pool_size=(2, 1), name='block3_pool'))

    model.add(tf.keras.layers.Flatten(name='flatten'))
    model.add(tf.keras.layers.Dense(units=128, activation='relu', name='fc1'))
    model.add(tf.keras.layers.Dropout(0.5, name='dropout1'))
    model.add(tf.keras.layers.Dense(units=128, activation='relu', name='fc2'))
    model.add(tf.keras.layers.Dropout(0.5, name='dropout2'))

    model.add(tf.keras.layers.Dense(units=OUTPUTS,
              activation='softmax', name='predictions'))

    opt = tf.keras.optimizers.Adam(learning_rate=learning_rate)
    model.compile(loss='sparse_categorical_crossentropy',
                  optimizer=opt, metrics=['accuracy'])

    model.fit(x_train, y_train, batch_size=batch_size,
              epochs=epochs, validation_split=0.2)
    test_loss, test_acc = model.evaluate(x_test, y_test, verbose=0)
    print("test_loss=", test_loss)
    print("test_acc=", test_acc)

    return model


def _load_training_data(base_dir):
    """Load MNIST training data"""
    x_train = np.load(os.path.join(base_dir, "train_data.npy"))
    y_train = np.load(os.path.join(base_dir, "train_labels.npy"))
    return x_train, y_train


def _load_testing_data(base_dir):
    """Load MNIST testing data"""
    x_test = np.load(os.path.join(base_dir, "eval_data.npy"))
    y_test = np.load(os.path.join(base_dir, "eval_labels.npy"))
    return x_test, y_test


def _parse_args():
    parser = argparse.ArgumentParser()

    # Data, model, and output directories
    # model_dir is always passed in from SageMaker. By default this is a S3 path under the default bucket.
    parser.add_argument('--epochs', type=int, default=10)
    parser.add_argument('--batch_size', type=int, default=64)
    parser.add_argument('--learning_rate', type=float, default=0.1)

    parser.add_argument("--model_dir", type=str)
    parser.add_argument("--sm-model-dir", type=str,
                        default=os.environ.get("SM_MODEL_DIR"))
    parser.add_argument("--train", type=str,
                        default=os.environ.get("SM_CHANNEL_TRAINING"))
    parser.add_argument('--test', type=str,
                        default=os.environ.get('SM_CHANNEL_TEST'))
    parser.add_argument("--hosts", type=list,
                        default=json.loads(os.environ.get("SM_HOSTS")))
    parser.add_argument("--current-host", type=str,
                        default=os.environ.get("SM_CURRENT_HOST"))

    return parser.parse_known_args()


if __name__ == '__main__':
    args, unknown = _parse_args()

    train_data, train_labels = _load_training_data(args.train)
    eval_data, eval_labels = _load_testing_data(args.train)

    batch_size = args.batch_size
    epochs = args.epochs
    learning_rate = args.learning_rate
    print(
        "batch_size = {}, epochs = {}, learning rate = {}".format(
            batch_size, epochs, learning_rate)
    )

    touch_classifier = model(train_data, train_labels, eval_data,
                             eval_labels, batch_size, epochs, learning_rate)

    if args.current_host == args.hosts[0]:
        # save model to an S3 directory with version number '00000001' in Tensorflow SavedModel Format
        # To export the model as h5 format use model.save('my_model.h5')
        touch_classifier.save(os.path.join(args.sm_model_dir, "000000001"))

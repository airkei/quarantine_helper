# some utility functions using boto3

import sys
import boto3
import pandas as pd
import io
import time
import datetime
#from matplotlib import pyplot as plt
import numpy as np
import math
from datetime import date, timedelta, timezone
import os


def get_s3_filelist(bucketnm):
    '''
    list all the files in the s3 bucket
    input the bucketnm 
    output a list of strings with file names
    '''
    s3 = boto3.resource('s3')
    bucket = s3.Bucket(bucketnm)
    return ([obj.key for obj in bucket.objects.all()])


def read_s3_file(bucketnm, filenm, encoding='utf8', sep=',', error_bad_lines=False):
    '''
    a query to read the file (csv file) in the s3 bucket
    input is bucketnm, filenm, encoding default is utf8, delimilater default is ,
    output is the dataframe
    '''
    s3client = boto3.client('s3')
    obj = s3client.get_object(Bucket=bucketnm, Key=filenm)
    df = pd.read_csv(io.BytesIO(
        obj['Body'].read()), encoding=encoding, sep=sep, error_bad_lines=False)
    return(df)


def write_file_to_s3(bucketnm, filenm, local_file_path):
    '''
    write a file to s3 bucket
    '''
    s3 = boto3.resource('s3')
    s3.Object(bucketnm, filenm).put(
        Body=open(local_file_path, 'rb'), ACL='bucket-owner-full-control')


def write_dir_to_s3(bucketnm, dirnm, local_dir_path):
    '''
    write a directory to s3 bucket
    '''
    parent_dir = os.path.dirname(os.path.realpath(local_dir_path))
    for (path, dirs, files) in os.walk(local_dir_path):
        for fn in files:
            if fn.startswith('.'):
                continue
            abspath = os.path.join(path, fn)
            write_file_to_s3(bucketnm, dirnm + '/' + abspath, abspath)


def read_s3_body(bucketnm, filenm):
    '''
    a query to read the file body in the s3 bucket
    input is bucketnm, filenm
    output is the data body
    '''
    s3 = boto3.resource('s3')
    body = s3.Bucket(bucketnm).Object(filenm).get()['Body'].read()
    return(body)


def read_s3_multiple_files(bucketnm, filelist):
    '''
    read all csv files in the s3 bucket
    assume that the files have the same dataframe structure
    '''
    df = pd.concat([read_s3_file(bucketnm, i) for i in filelist])
    return(df)


def read_s3_allfiles(bucketnm):
    '''
    read all files in the s3 bucket
    input is the bucket name
    assume that the files are csv format with the same structure
    '''
    filelist = get_s3_filelist(bucketnm)
    df = read_s3_multiple_files(bucketnm, filelist)
    return(df)


def write_df_to_csv_on_s3(bucketnm, df, filename):
    '''
    Write a dataframe to a CSV on S3 
    input the bucketnm (string), df (dataframe), and filename (string)
    '''
    csv_buffer = io.StringIO()
    # Write dataframe to buffer
    df.to_csv(csv_buffer, sep=",", index=False)
    s3 = boto3.resource("s3")
    # Write buffer to S3 object
    s3.Object(bucketnm, filename).put(
        Body=csv_buffer.getvalue(), ACL='bucket-owner-full-control')


def append_df_to_csv_on_s3(bucketnm, df, filename):
    try:
        df_old = read_s3_file(bucketnm, filenm=filename)
    except:
        print('file does not exist')
        df_old = None
    df_new = pd.concat([df_old, df])
    write_df_to_csv_on_s3(bucketnm, df_new, filename)

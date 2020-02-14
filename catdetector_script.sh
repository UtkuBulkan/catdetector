export GOOGLE_APPLICATION_CREDENTIALS='/catdetector_gcloud_certificates/service-cert/supereye.co.uk-storage.json'
gcloud auth activate-service-account --key-file /catdetector_gcloud_certificates/service-cert/supereye.co.uk-storage.json
gcsfuse --implicit-dirs supereye.co.uk /catdetector/release/data
cd /catdetector/release 
#./catdetector data/bulkanutku-1/birds.mp4/birds.mp4 birds.mp4 bulkanutku-1
./catdetector $catdetector_path $catdetector_uuid $catdetector_user
